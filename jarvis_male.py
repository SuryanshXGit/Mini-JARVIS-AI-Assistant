import speech_recognition as sr
import subprocess
import json
import os
import re
import threading
import time
import winsound
import webbrowser
from datetime import datetime, timedelta

# ==========================================================
# MINI JARVIS - MALE VOICE VERSION
# ==========================================================
# Install in VS Code terminal:
# py -m pip install SpeechRecognition PyAudio
#
# VOICE COMMANDS:
# Hello Jarvis
# How are you?
# What is your name?
# What time is it?
# What is the date?
# Open Notepad
# Open Calculator
# Open VS Code
# Open YouTube
# Open Google
# Add task finish my C assignment
# Remember to submit my assignment
# Show tasks
# Complete task 1
# Delete task 1
# Set alarm for 7:30 AM
# Set alarm for 10 PM
# Set alarm at 18:45
# Show alarms
# Cancel alarm 1
# Stop alarm
# Go to sleep
# Wake up Jarvis
# Stop listening
# ==========================================================

BASE = os.path.dirname(os.path.abspath(__file__))
TASK_FILE = os.path.join(BASE, "tasks.json")
ALARM_FILE = os.path.join(BASE, "alarms.json")

recognizer = sr.Recognizer()
recognizer.dynamic_energy_threshold = True
recognizer.energy_threshold = 300
recognizer.pause_threshold = 0.7

running = True
sleeping = False
alarm_ringing = False
alarm_stop_event = threading.Event()
data_lock = threading.Lock()


# ---------------- MALE VOICE OUTPUT ----------------

def speak(message):
    print(f"\nJARVIS: {message}")

    safe = message.replace("'", "''")

    script = (
        "Add-Type -AssemblyName System.Speech; "
        "$s=New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$v=$s.GetInstalledVoices() | "
        "Where-Object {$_.VoiceInfo.Name -match "
        "'David|Guy|Mark|Ryan|George|James|Richard'}; "
        "if($v){$s.SelectVoice($v[0].VoiceInfo.Name)}; "
        f"$s.Speak('{safe}'); "
        "$s.Dispose()"
    )

    try:
        subprocess.run(
            ["powershell.exe", "-NoProfile", "-Command", script],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=40
        )
    except Exception as e:
        print("Voice output error:", e)


# ---------------- SAVE AND LOAD DATA ----------------

def load_data(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            value = json.load(f)
            return value if isinstance(value, list) else []
    except (OSError, json.JSONDecodeError):
        return []


def save_data(path, value):
    temp = path + ".tmp"

    with open(temp, "w", encoding="utf-8") as f:
        json.dump(value, f, indent=2, ensure_ascii=False)

    os.replace(temp, path)


# ---------------- VOICE RECOGNITION ----------------

def listen(source):
    try:
        print("\nListening...")

        audio = recognizer.listen(
            source,
            timeout=8,
            phrase_time_limit=8
        )

        command = recognizer.recognize_google(
            audio,
            language="en-IN"
        ).lower().strip()

        print("YOU:", command)
        return command

    except sr.WaitTimeoutError:
        return ""

    except sr.UnknownValueError:
        print("Didn't understand the audio.")
        return ""

    except sr.RequestError:
        print("Speech recognition needs internet.")
        time.sleep(2)
        return ""

    except Exception as e:
        print("Microphone/listening error:", e)
        time.sleep(1)
        return ""


# ---------------- TASK MANAGEMENT ----------------

def add_task(title):
    title = title.strip(" .,!?")

    if not title:
        speak("Please say the task after the words add task.")
        return

    with data_lock:
        tasks = load_data(TASK_FILE)

        tasks.append({
            "title": title,
            "done": False
        })

        save_data(TASK_FILE, tasks)
        number = len(tasks)

    speak(f"Task {number} added: {title}.")


def task_number(command):
    match = re.search(r"\b(\d+)\b", command)

    if match:
        return int(match.group(1))

    words = {
        "one": 1, "first": 1,
        "two": 2, "second": 2,
        "three": 3, "third": 3,
        "four": 4, "fourth": 4,
        "five": 5, "fifth": 5,
        "six": 6, "sixth": 6,
        "seven": 7, "eight": 8,
        "nine": 9, "ten": 10
    }

    for word, number in words.items():
        if re.search(rf"\b{word}\b", command):
            return number

    return None


def show_tasks():
    tasks = load_data(TASK_FILE)

    if not tasks:
        speak("You have no saved tasks.")
        return

    messages = []

    for i, task in enumerate(tasks[:10]):
        status = "completed" if task.get("done") else "pending"

        messages.append(
            f"Task {i + 1}: {task.get('title', 'Untitled')}, {status}"
        )

    speak(". ".join(messages))


def change_task(command, complete):
    number = task_number(command)

    with data_lock:
        tasks = load_data(TASK_FILE)

        if number is None or number < 1 or number > len(tasks):
            speak("I couldn't find that task number. Say show tasks.")
            return

        if complete:
            tasks[number - 1]["done"] = True
            response = f"Task {number} marked complete."

        else:
            removed = tasks.pop(number - 1)

            response = (
                f"Deleted task {number}: "
                f"{removed.get('title', 'Untitled')}."
            )

        save_data(TASK_FILE, tasks)

    speak(response)


# ---------------- ALARM TIME PARSING ----------------

def parse_alarm_time(command):
    # Supports:
    # 7:30 AM
    # 10 PM
    # 18:45
    # 7 AM

    text = command.lower().replace(".", "")

    # First look for a time containing minutes.
    match = re.search(
        r"\b(\d{1,2})\s*:\s*(\d{2})\s*(am|pm)?\b",
        text
    )

    if match:
        hour = int(match.group(1))
        minute = int(match.group(2))
        period = match.group(3)

    else:
        # Look for a time such as 10 PM or 7 AM.
        match = re.search(
            r"\b(\d{1,2})\s*(am|pm)\b",
            text
        )

        if match:
            hour = int(match.group(1))
            minute = 0
            period = match.group(2)

        else:
            # Also accept whole-hour 24-hour times.
            match = re.search(
                r"\b(?:for|at)\s+(\d{1,2})\b",
                text
            )

            if not match:
                return None

            hour = int(match.group(1))
            minute = 0
            period = None

    if not 0 <= minute <= 59:
        return None

    if period:
        if not 1 <= hour <= 12:
            return None

        if period == "pm" and hour != 12:
            hour += 12

        elif period == "am" and hour == 12:
            hour = 0

    elif not 0 <= hour <= 23:
        return None

    now = datetime.now()

    target = now.replace(
        hour=hour,
        minute=minute,
        second=0,
        microsecond=0
    )

    # If this time has already passed, schedule it for tomorrow.
    if target <= now:
        target += timedelta(days=1)

    return target


# ---------------- SET ALARM ----------------

def set_alarm(command):
    target = parse_alarm_time(command)

    if target is None:
        speak(
            "I couldn't understand the alarm time. "
            "Say set alarm for 7:30 AM, or set alarm for 10 PM."
        )
        return

    with data_lock:
        alarms = load_data(ALARM_FILE)

        alarm_id = max(
            (int(a.get("id", 0)) for a in alarms),
            default=0
        ) + 1

        alarms.append({
            "id": alarm_id,
            "time": target.isoformat(timespec="seconds"),
            "fired": False
        })

        save_data(ALARM_FILE, alarms)

    speak(
        f"Alarm {alarm_id} is set for "
        f"{target.strftime('%I:%M %p')}."
    )


def show_alarms():
    alarms = load_data(ALARM_FILE)
    active = []

    for alarm in alarms:
        if not alarm.get("fired", False):
            try:
                when = datetime.fromisoformat(alarm["time"])

                active.append(
                    f"Alarm {alarm['id']} at "
                    f"{when.strftime('%I:%M %p on %B %d')}"
                )

            except (ValueError, KeyError, TypeError):
                pass

    if active:
        speak(". ".join(active))
    else:
        speak("You have no active alarms.")


def cancel_alarm(command):
    number = task_number(command)

    if number is None:
        speak("Say cancel alarm followed by its number.")
        return

    with data_lock:
        alarms = load_data(ALARM_FILE)

        remaining = [
            alarm for alarm in alarms
            if int(alarm.get("id", -1)) != number
        ]

        if len(remaining) == len(alarms):
            speak(f"I couldn't find alarm {number}.")
            return

        save_data(ALARM_FILE, remaining)

    speak(f"Alarm {number} cancelled.")


# ---------------- STOP RINGING ALARM ----------------

def stop_alarm():
    global alarm_ringing

    if alarm_ringing:
        alarm_stop_event.set()
        alarm_ringing = False
        speak("Alarm stopped.")
    else:
        speak("No alarm is ringing right now.")


# ---------------- BACKGROUND ALARM CHECKER ----------------

def alarm_worker():
    global alarm_ringing

    while running:
        try:
            now = datetime.now()
            due_alarm = None

            with data_lock:
                alarms = load_data(ALARM_FILE)

                for alarm in alarms:
                    if alarm.get("fired", False):
                        continue

                    try:
                        target = datetime.fromisoformat(alarm["time"])

                        if now >= target:
                            alarm["fired"] = True
                            due_alarm = alarm

                    except (ValueError, KeyError, TypeError):
                        continue

                save_data(ALARM_FILE, alarms)

            if due_alarm is not None:
                alarm_stop_event.clear()
                alarm_ringing = True

                threading.Thread(
                    target=speak,
                    args=(
                        f"Attention. Alarm {due_alarm['id']} is ringing.",
                    ),
                    daemon=True
                ).start()

                print(
                    f"\n*** ALARM {due_alarm['id']} RINGING "
                    f"- say 'stop alarm' ***"
                )

                while running and not alarm_stop_event.is_set():
                    winsound.Beep(1000, 700)

                    if alarm_stop_event.wait(0.25):
                        break

                    winsound.Beep(1300, 700)

                alarm_ringing = False

        except Exception as e:
            print("Alarm worker error:", e)

        time.sleep(1)


# ---------------- OPEN APPS AND WEBSITES ----------------

def open_app(command):
    try:
        if "notepad" in command:
            subprocess.Popen(["notepad.exe"])
            speak("Opening Notepad.")

        elif "calculator" in command or "calc" in command:
            subprocess.Popen(["calc.exe"])
            speak("Opening Calculator.")

        elif "vs code" in command or "visual studio code" in command:
            subprocess.Popen(["code"])
            speak("Opening Visual Studio Code.")

        elif "youtube" in command:
            webbrowser.open("https://www.youtube.com")
            speak("Opening YouTube.")

        elif "google" in command:
            webbrowser.open("https://www.google.com")
            speak("Opening Google.")

    except OSError:
        speak("I couldn't open that app. Check that it is installed.")


# ---------------- COMMAND HANDLER ----------------

def handle_command(command):
    global running, sleeping

    if not command:
        return

    # Alarm stopping must work even when Jarvis is in sleep mode.
    if "stop alarm" in command or "stop the alarm" in command:
        stop_alarm()
        return

    if any(phrase in command for phrase in (
        "shutdown jarvis",
        "stop listening",
        "exit jarvis",
        "quit jarvis"
    )):
        running = False
        speak("Shutting down. Goodbye.")
        return

    if "go to sleep" in command or "sleep mode" in command:
        sleeping = True
        speak("Going to sleep. Say wake up Jarvis to continue.")
        return

    if "wake up jarvis" in command or command == "wake up":
        sleeping = False
        speak("I'm awake and listening.")
        return

    if sleeping:
        return

    # Alarm commands: extract and save the spoken time.
    if (
        "alarm" in command
        and any(word in command for word in ("set", "create", "add", "for", "at"))
        and not any(word in command for word in (
            "show", "list", "cancel", "delete", "stop"
        ))
    ):
        set_alarm(command)
        return

    if "show alarms" in command or "list alarms" in command:
        show_alarms()
        return

    if "cancel alarm" in command or "delete alarm" in command:
        cancel_alarm(command)
        return

    # Add tasks by speaking.
    if any(phrase in command for phrase in (
        "add task",
        "create task",
        "remember to",
        "remind me to"
    )):
        title = command

        prefixes = (
            "please add task",
            "add a task",
            "add task",
            "create a task",
            "create task",
            "remember to",
            "remind me to"
        )

        for prefix in prefixes:
            if title.startswith(prefix):
                title = title[len(prefix):].strip()
                break

        add_task(title)
        return

    if (
        "show tasks" in command
        or "list tasks" in command
        or "my tasks" in command
    ):
        show_tasks()
        return

    if any(phrase in command for phrase in (
        "complete task",
        "finish task",
        "mark task"
    )):
        change_task(command, True)
        return

    if "delete task" in command or "remove task" in command:
        change_task(command, False)
        return

    # Time and date.
    if "what time" in command or "tell me the time" in command:
        speak("It is " + datetime.now().strftime("%I:%M %p"))

    elif (
        "what is the date" in command
        or "today's date" in command
        or "what date" in command
    ):
        speak("Today is " + datetime.now().strftime("%A, %B %d, %Y"))

    elif any(word in command for word in (
        "hello jarvis", "hey jarvis", "hi jarvis", "hello", "hey"
    )):
        speak("Hello. I'm listening. What can I do for you?")

    elif "how are you" in command:
        speak("I'm doing great, thank you for asking.")

    elif "your name" in command or "who are you" in command:
        speak("I am your Mini Jarvis voice assistant.")

    elif "who created you" in command:
        speak("You are building me in Python.")

    elif "what can you do" in command or command == "help":
        speak(
            "I can set alarms, manage tasks, tell the time and date, "
            "and open apps and websites."
        )

    elif command.startswith("open ") and any(app in command for app in (
        "notepad", "calculator", "calc", "vs code",
        "visual studio code", "youtube", "google"
    )):
        open_app(command)

    else:
        speak("Sorry, I didn't understand that command. Please say it again.")


# ---------------- MAIN PROGRAM ----------------

def main():
    global running

    speak("Mini Jarvis is starting. Connecting to your microphone.")

    try:
        microphone = sr.Microphone()

    except Exception as e:
        print("Microphone initialization failed:", e)
        speak(
            "I cannot access your microphone. "
            "Check Windows microphone permissions and PyAudio."
        )
        return

    threading.Thread(
        target=alarm_worker,
        daemon=True
    ).start()

    try:
        with microphone as source:
            print("Calibrating microphone. Stay quiet for one second...")
            recognizer.adjust_for_ambient_noise(source, duration=1)

        speak("I'm ready. You can speak your commands.")

    except Exception as e:
        print("Microphone calibration failed:", e)
        speak("I could not calibrate the microphone. Please check your microphone.")
        return

    while running:
        try:
            with microphone as source:
                command = listen(source)

            if command:
                handle_command(command)

        except KeyboardInterrupt:
            running = False

        except Exception as e:
            print("Main loop error:", e)
            time.sleep(1)

    print("Mini JARVIS stopped.")


if __name__ == "__main__":
    main()