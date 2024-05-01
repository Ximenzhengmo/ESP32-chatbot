import http.server
import socketserver
import json
import base64
import wave
import io
import requests

"""
    transit server to receive audio data from ESP32, and send it to Baidu ASR service for debug use
"""


PORT = 8000
wav_path = "" # save the audio file recorded by ESP32

API_KEY = ""  # your Baidu ASR access key
SECRET_KEY = ""  # your Baidu ASR secret key

def asr(speech64, length):
    url = "https://vop.baidu.com/pro_api"

    payload = json.dumps({
        "format": "pcm",
        "rate": 16000,
        "channel": 1,
        "cuid": "ximenzhengmo",
        "token": get_access_token(),
        "dev_pid": 80001,
        "speech": speech64,
        "len": length,
    })
    headers = {
        'Content-Type': 'application/json',
        'Accept': 'application/json'
    }

    response = requests.request("POST", url, headers=headers, data=payload)

    print(response.text)
    return response.text


def get_access_token():
    """
    use AK, SK get (Access Token)
    :return: access_token, or None(if error)
    """
    url = "https://aip.baidubce.com/oauth/2.0/token"
    params = {"grant_type": "client_credentials", "client_id": API_KEY, "client_secret": SECRET_KEY}
    return str(requests.post(url, params=params).json().get("access_token"))


# create transit server
class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        # set response header
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()

        # get content
        all_content = ''
        while True:
            data = self.rfile.readline().strip().decode('utf-8')
            data = self.rfile.readline().strip().decode('utf-8')
            if not data:
                break
            print(f"Received POST request with data: {data}")
            all_content += data
        print("Received ALL POST request")
        json_content = json.loads(all_content)
        speech = json_content['speech']
        decode_speech = base64.b64decode(speech)
        with io.BytesIO(decode_speech) as memfile:
            # open `wave` module to write audio data
            with wave.open(memfile, "wb") as wavfile:
                wavfile.setnchannels(1)  # channel_num
                wavfile.setsampwidth(2)  # set bit width ( 2 bytes )
                wavfile.setframerate(16000)  # set sample rate
                wavfile.writeframes(decode_speech)  # write audio data

            # write binary data to file
            with open(wav_path, "wb") as file:
                file.write(memfile.getvalue())
        # response to ESP32
        result = asr(speech, json_content['len'])
        self.wfile.write(result.encode('utf-8'))

# create server
with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
    print(f"Serving at port {PORT}")
    # start server
    httpd.serve_forever()