
Add-Type -AssemblyName System.Speech

$ErrorActionPreference = "Stop"

$voice = $null
$recognizer = $null
$running = $true

try {
    # Find an installed English speech recognizer
    $installed = @(
        [System.Speech.Recognition.SpeechRecognitionEngine]::InstalledRecognizers()
    )

    $selected = $installed |
        Where-Object {
            $_.Culture.Name -like "en-*"
        } |
        Select-Object -First 1

    if ($null -eq $selected) {
        throw "No English speech recognizer is installed. Install an English speech recognition language in Windows."
    }

    Write-Host "Using recognizer: $($selected.Name)" -ForegroundColor Green

    # Initialize speech output
    $voice = New-Object System.Speech.Synthesis.SpeechSynthesizer
    $voice.Volume = 100
    $voice.Rate = 0

    $installedVoice = $voice.GetInstalledVoices() |
        Where-Object {
            $_.Enabled -and $_.VoiceInfo.Culture.Name -like "en-*"
        } |
        Select-Object -First 1

    if ($installedVoice) {
        $voice.SelectVoice($installedVoice.VoiceInfo.Name)
    }

    # Initialize microphone recognition
    $recognizer = New-Object `
        System.Speech.Recognition.SpeechRecognitionEngine($selected)

    $commands = @(
        "Jarvis",
        "Hey Jarvis",
        "Hello",
        "Hello Jarvis",
        "Wake up",
        "How are you",
        "What is your name",
        "Who created you",
        "What can you do",
        "What time is it",
        "Tell me the date",
        "Open Notepad",
        "Open Calculator",
        "Open YouTube",
        "Open Google",
        "Open VS Code",
        "Show notes",
        "Show tasks",
        "Show timetable",
        "Thank you",
        "Go to sleep",
        "Stop listening"
    )

    $choices = New-Object System.Speech.Recognition.Choices
    $choices.Add([string[]]$commands)

    $builder = New-Object System.Speech.Recognition.GrammarBuilder
    $builder.Culture = $selected.Culture
    $builder.Append($choices)

    $grammar = New-Object System.Speech.Recognition.Grammar($builder)
    $recognizer.LoadGrammar($grammar)

    # Display rejected phrases instead of silently ignoring them
    $recognizer.add_SpeechRecognitionRejected({
        param($sender, $e)
        Write-Host "Speech not matched to a command. Try speaking clearly." `
            -ForegroundColor Yellow
    })

    # Use the Windows default microphone
    $recognizer.SetInputToDefaultAudioDevice()

    function Say([string]$message) {
        Write-Host "JARVIS: $message" -ForegroundColor Cyan
        $voice.Speak($message)
    }

    function Launch([string]$app, [string]$message) {
        try {
            Start-Process $app
            Say $message
        }
        catch {
            Say "Sorry sir, I could not open that application."
        }
    }

    Write-Host "`nJARVIS IS ONLINE" -ForegroundColor Green
    Write-Host "Speak one command at a time." -ForegroundColor Green

    Say "JARVIS is online, sir. I am listening."

    while ($running) {
        # Wait for the next recognized utterance
        $result = $recognizer.Recognize()

        if ($null -eq $result) {
            continue
        }

        $phrase = $result.Text.Trim().ToLowerInvariant()

        Write-Host "YOU: $phrase" -ForegroundColor White

        if ($result.Confidence -lt 0.45) {
            Say "Sorry sir, please repeat that."
            continue
        }

        switch ($phrase) {
            { $_ -in "jarvis", "hey jarvis", "wake up" } {
                Say "Yes sir. I am listening."
                break
            }
            { $_ -in "hello", "hello jarvis" } {
                Say "Hello sir. How can I help you?"
                break
            }
            "how are you" {
                Say "I am functioning well, sir."
                break
            }
            "what is your name" {
                Say "I am JARVIS, your personal assistant."
                break
            }
            "who created you" {
                Say "You are building me, sir."
                break
            }
            "what can you do" {
                Say "I can respond to supported voice commands and open applications."
                break
            }
            "what time is it" {
                Say "The time is $(Get-Date -Format 'h:mm tt'), sir."
                break
            }
            "tell me the date" {
                Say "Today is $(Get-Date -Format 'dddd, dd MMMM yyyy'), sir."
                break
            }
            "open notepad" {
                Launch "notepad.exe" "Opening Notepad, sir."
                break
            }
            "open calculator" {
                Launch "calc.exe" "Opening Calculator, sir."
                break
            }
            "open youtube" {
                Start-Process "https://www.youtube.com"
                Say "Opening YouTube, sir."
                break
            }
            "open google" {
                Start-Process "https://www.google.com"
                Say "Opening Google, sir."
                break
            }
            "open vs code" {
                Launch "code" "Opening Visual Studio Code, sir."
                break
            }
            "show notes" {
                Say "The notes command is recognized, but your notes module is not connected yet."
                break
            }
            "show tasks" {
                Say "The tasks command is recognized, but your tasks module is not connected yet."
                break
            }
            "show timetable" {
                Say "The timetable command is recognized, but your timetable module is not connected yet."
                break
            }
            "thank you" {
                Say "You are welcome, sir."
                break
            }
            "go to sleep" {
                Say "Going to sleep, sir."
                break
            }
            "stop listening" {
                Say "Goodbye, sir. Shutting down."
                $running = $false
                break
            }
            default {
                Say "Sorry sir, that command is not available."
            }
        }
    }
}
catch {
    Write-Host "`nJARVIS ERROR: $($_.Exception.Message)" -ForegroundColor Red
}
finally {
    if ($recognizer) { $recognizer.Dispose() }
    if ($voice) { $voice.Dispose() }

    Write-Host "JARVIS has stopped." -ForegroundColor Yellow
}