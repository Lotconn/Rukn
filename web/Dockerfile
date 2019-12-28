FROM python:3

WORKDIR /app
COPY . .
RUN python -m pip install -r requirements.txt
EXPOSE 8000
CMD ["gunicorn", "-w", "2", "-b", "0.0.0.0:8000", "munadi_org:app"]
