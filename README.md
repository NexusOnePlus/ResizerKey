# ResizerKey
![alt text](resizekey256.ico)


ResizerKey is a Windows utility that allows you to easily resize and move windows using keyboard shortcuts. This application runs in the background and provides quick access to window resizing functions without the need to use the mouse.

## Main Features

- **Quick Resize**: Resizes the active window to occupy most of the screen (with margin) using Win+B
- **Precision Resize Mode**: Activate a special mode with Win+Shift+B that allows you to:
  - Move the window using arrow keys
  - Change the window size using specific keys
- **Lightweight**: Consumes minimal system resources
- **Status Indicator**: Displays a small overlay when resize mode is active

## Keyboard Shortcuts

### Basic Commands:
- **Win+B**: Automatically resize the active window to occupy most of the screen (with a 20px margin)
- **Win+Shift+B**: Activate precision resize mode
- **ESC**: Exit precision resize mode

### In Precision Resize Mode:
- **Arrow Keys** (←↑↓→): Move the window
- **J**: Expand to the left
- **Ñ**: Expand to the right
- **K**: Expand upward
- **L**: Expand downward

- **U**: Contract from the left
- **P**: Contract from the right
- **I**: Contract from the top
- **O**: Contract from the bottom

## Installation

1. Download the latest release from the releases page
2. Run the executable
3. A tray icon will appear in your system tray indicating the application is running

## Exiting the Application

Click on the tray icon to exit the application.

## System Requirements

- Windows 7/8/10/11
- Minimal system resources required

## Building from Source

If you want to build ResizerKey from source:

1. Ensure you have a C++ compiler (like MinGW-w64/g++)
2. Compile using these commands
    ```
    windres resizerkey.rc -O coff -o resizerkey_res.o
    g++ -mwindows resizerkey.cpp resizerkey_res.o -o ResizerKey.exe -lgdi32
    ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Feel free to submit issues or pull requests to improve ResizerKey.
