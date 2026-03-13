import io, os, logging
from datetime import datetime
from fastapi import FastAPI, Request, Response
from pydub import AudioSegment
from groq import Groq
import edge_tts, uvicorn
from dotenv import load_dotenv

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

load_dotenv()
VOICE_ID = "id-ID-ArdiNeural"
GROQ_API_KEY = os.getenv("GROQ_API_KEY")
client = Groq(api_key=GROQ_API_KEY)
app = FastAPI()

RECORDINGS_DIR = "recordings"
if not os.path.exists(RECORDINGS_DIR): os.makedirs(RECORDINGS_DIR)

SYSTEM_PROMPT = {"role": "system", "content": "Kamu robot bernama Bodot. Jawab sangat singkat dan santai"}
history = [SYSTEM_PROMPT]

def process_text_with_ai(user_text: str) -> str:
    global history
    text_lower = user_text.lower()
    
    # Filter Keyword agar tidak menjawab noise sembarang
    if "terima kasih" in text_lower:
        logger.info(f"Noise!")
        return ""

    clean_prompt = text_lower.replace("bodot", "").strip()
    if not clean_prompt: return "Ya bos? Ada yang bisa Bodot bantu? HAHA"

    history.append({"role": "user", "content": clean_prompt})
    if len(history) > 11: history.pop(1); history.pop(1)

    try:
        chat = client.chat.completions.create(
            messages=history, model="llama-3.3-70b-versatile", temperature=0.7, max_tokens=150
        )
        reply = chat.choices[0].message.content.strip()
        history.append({"role": "assistant", "content": reply})
        return reply
    except Exception as e:
        logger.error(f"AI Error: {e}"); return "Aduh, otak Bodot korslet."

@app.post("/process-audio")
async def process_audio(request: Request):
    audio_data = bytearray()
    
    # try-except untuk menangani ClientDisconnect dari ESP32
    try:
        async for chunk in request.stream(): 
            audio_data.extend(chunk)
    except Exception as e:
        logger.error(f"Koneksi terputus saat streaming: {e}")
        return Response(status_code=204)
    
    # Cek apakah data cukup untuk diproses
    if len(audio_data) < 2000: 
        return Response(status_code=204)

    # 1. Load Audio dari data mentah (RAW)
    audio = AudioSegment.from_file(
        io.BytesIO(audio_data), 
        format="raw", 
        sample_width=2, 
        frame_rate=16000, 
        channels=1
    )

    # SIMPAN REKAMAN (Untuk Audit/Cek Noise)
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{RECORDINGS_DIR}/rec_{ts}.wav"
    audio.export(filename, format="wav")
    logger.info(f"Rekaman disimpan: {filename}")

    # Persiapkan Buffer untuk Whisper AI
    wav_buf = io.BytesIO()
    audio.export(wav_buf, format="wav")
    wav_buf.seek(0)

    # STT (Speech To Text)
    try:
        transcription = client.audio.transcriptions.create(
            file=("input.wav", wav_buf.read()),
            model="whisper-large-v3-turbo",
            language="id"
        )
        user_text = transcription.text.strip()
    except Exception as e:
        logger.error(f"STT Error: {e}")
        return Response(status_code=204)

    # AI Logic (Filter Keyword "Bodot")
    reply = process_text_with_ai(user_text)
    
    # Jika transkripsi hanya noise dan tidak mengandung "Bodot", abaikan
    if not reply: 
        return Response(status_code=204)

    logger.info(f"input: {user_text} | bodot: {reply}")

    # 5. TTS (Text To Speech)
    tts_path = f"temp_{ts}.mp3"
    try:
        tts = edge_tts.Communicate(reply, VOICE_ID)
        await tts.save(tts_path)
        
        tts_audio = AudioSegment.from_mp3(tts_path)
        tts_audio = tts_audio.set_frame_rate(16000).set_channels(1).set_sample_width(2)
        
        out_buf = io.BytesIO()
        tts_audio.export(out_buf, format="s16le")
        if os.path.exists(tts_path): os.remove(tts_path)

        # Ganti karakter newline (\n) dan carriage return (\r) menjadi spasi
        clean_reply = reply.replace("\n", " ").replace("\r", " ").strip()

        # Pastikan hanya karakter ASCII yang dikirim untuk menghindari error encoding
        clean_reply = clean_reply.encode('ascii', 'ignore').decode('ascii')

        return Response(
            content=out_buf.read(),
            media_type="application/octet-stream",
            headers={
                "X-Transcription": user_text[:100].replace("\n", " "),
                "X-Reply": clean_reply[:150] # Tetap potong untuk keamanan OLED
            }
        )
    
    except Exception as e:
        logger.error(f"TTS Error: {e}")
        return Response(status_code=204)
    
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8010)
