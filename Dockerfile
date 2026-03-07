FROM python:3.11-slim

WORKDIR /app

# Instal dependensi sistem (ffmpeg)
RUN apt-get update && apt-get install -y --no-install-recommends \
    ffmpeg \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# LANGKAH KRUSIAL: Copy requirements dulu
COPY requirements.txt .

# Instal dependensi Python
# Karena requirements.txt jarang berubah, layer ini akan di-cache oleh Docker
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Baru copy seluruh source code setelah instalasi library selesai
COPY . .

# Buat folder recordings
RUN mkdir -p recordings

CMD ["python", "main.py"]
