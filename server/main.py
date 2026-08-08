import mysql.connector
import time
from datetime import datetime
import paho.mqtt.client as mqtt
from config import MQTT_BROKER, MQTT_PORT, DB_CONFIG

court_states = {i: None for i in range(1, 11)}

def get_mqtt_client():
    client = mqtt.Client()
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        return client
    except Exception as e:
        print(f"[Error] Connect MQTT Failed: {e}")
        return None

def check_bookings():
    client = get_mqtt_client()
    if not client: return 

    now = datetime.now()
    current_time_str = now.strftime("%Y-%m-%d %H:%M:%S")

    conn = None
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()

        query = "SELECT court_id FROM bookings WHERE %s BETWEEN start_time AND end_time"
        cursor.execute(query, (current_time_str,))
        active_courts = [row[0] for row in cursor.fetchall()]

        if active_courts:
            print(f"At {current_time_str} Courts Active: {active_courts}")

        for court_id in range(1, 11):
            if court_id in active_courts:
                payload = "ON"
            else:
                payload = "OFF"

            if court_states[court_id] != payload:
                topic = f"court/{court_id}/control/light"
                client.publish(topic, payload)
                court_states[court_id] = payload
                print(f"UPDATE: Court {court_id} -> {payload}")
                time.sleep(0.05)

    except Exception as e:
        print(f"[Error] {e}")
    finally:
        if conn and conn.is_connected():
            cursor.close()
            conn.close()

        time.sleep(0.5)
        if client:
            client.loop_stop()
            client.disconnect()

if __name__ == "__main__":
    print("System Started (Smart Silent Mode)...")
    while True:
        check_bookings()
        time.sleep(10)
