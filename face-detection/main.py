from flask import Flask, render_template, Response
import cv2
import socket
import paho.mqtt.client as mqtt

app = Flask(__name__)

face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

functionMode = 1


mqtt_broker_address = "broker.hivemq.com"
mqtt_topic = "face-detection"

last_received_message = None

def on_message(client, userdata, message):
    global functionMode, last_received_message
    payload = message.payload.decode()
    print("message arrived:", payload)

    if payload == "1":
        functionMode = 1
        print("function mode: on")
    elif payload == "2":
        if payload != last_received_message:
            last_received_message = payload
            functionMode = 0
            print("function mode: off")
            print("``````")

def on_connect(client, userdata, flags, rc):
    global topic

    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(mqtt_topic)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Disconnected from MQTT broker")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.on_disconnect = on_disconnect
def detect_faces():
    global functionMode
    cap = cv2.VideoCapture(0)
    while True:
        ret, frame = cap.read()
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, scaleFactor=1.3, minNeighbors=5, minSize=(30, 30))
        if functionMode == 1:
            for (x, y, w, h) in faces:
                cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
                
            if len(faces) > 0:
                mqtt_client.publish(mqtt_topic, "2")
                functionMode = 0
                print("function mode: off")
        ret, jpeg = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + jpeg.tobytes() + b'\r\n')

@app.route('/')

@app.route('/video_feed')
def video_feed():
    return Response(detect_faces(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    mqtt_client.connect(mqtt_broker_address, port=1883, keepalive=60)
    mqtt_client.loop_start()
    
    app.run(host='0.0.0.0', port=8000, debug=True)
