"""Arduino Tools.

This module provides action utilities for the Arduino agent.
"""

import http.client
import json
import os

from langchain_core.tools import tool


def _send_arduino_request(tool_name, text=None):
    """Sends a request to the Arduino board using IP/host from env variable ARDUINO_IP."""
    arduino_ip = os.getenv("ARDUINO_IP")
    if not arduino_ip:
        return "Error: Arduino IP or host needs to be configured in the ARDUINO_IP environment variable."

    headers = {"Content-Type": "application/json"}
    data = {"tool_name": tool_name}
    if text:
        data["text"] = text

    body = json.dumps(data)

    try:
        # Reduced timeout to 2 seconds for faster feedback
        conn = http.client.HTTPConnection(arduino_ip, 80, timeout=2)
        conn.request("POST", "/mcp", body, headers)
        response = conn.getresponse()
        response_body = response.read().decode()
        conn.close()
        return response_body
    except TimeoutError:
        return "Error: Connection to Arduino timed out. Is the device online?"
    except ConnectionRefusedError:
        return "Error: Connection refused by Arduino. Is the server running?"
    except Exception as e:
        return f"Error: Could not connect to Arduino ({e}). Please check if the device is powered on and connected to the network."


@tool
def start_led_blinker() -> str:
    """Starts the LED blinker on the Arduino board.
    It's useful for providing a visual indication that a process is running.
    """
    return _send_arduino_request("start_led_blinker")


@tool
def play_sound() -> str:
    """Plays a sound on the Arduino board.
    It's useful for providing an audible notification.
    """
    return _send_arduino_request("play_sound")


@tool
def display_countdown() -> str:
    """Displays a countdown on an attached display.
    It's useful for timing events.
    """
    return _send_arduino_request("display_countdown")


@tool
def display_text(text: str) -> str:
    """Displays a string of text on an attached display.
    It's useful for showing messages to the user.
    """
    return _send_arduino_request("display_text", text)
