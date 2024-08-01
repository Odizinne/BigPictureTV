import winreg

# Map language codes to Big Picture mode window titles
BIG_PICTURE_WINDOW_TITLES = {
    "schinese": "Steam 大屏幕模式",
    "tchinese": "Steam Big Picture 模式",
    "japanese": "Steam Big Pictureモード",
    "koreana": "Steam Big Picture 모드",
    "thai": "โหมด Big Picture บน Steam",
    "bulgarian": "Steam режим „Голям екран“",
    "czech": "Steam režim Big Picture",
    "danish": "Steam Big Picture-tilstand",
    "german": "Big-Picture-Modus",
    "english": "Steam Big Picture mode",
    "spanish": "Modo Big Picture de Steam",
    "latam": "Modo Big Picture de Steam",
    "greek": "Steam Λειτουργία Big Picture",
    "french": "Steam mode Big Picture",
    "indonesian": "Mode Big Picture Steam",
    "italian": "Modalità Big Picture di Steam",
    "hungarian": "Steam Nagy Kép mód",
    "dutch": "Steam Big Picture-modus",
    "norwegian": "Steam Big Picture-modus",
    "polish": "Tryb Big Picture Steam",
    "portuguese": "Steam Big Picture",
    "brazilian": "Steam Modo Big Picture",
    "romanian": "Steam modul Big Picture",
    "russian": "Режим Big Picture",
    "finnish": "Steamin televisiotila",
    "swedish": "Steams Big Picture-läge",
    "turkish": "Steam Geniş Ekran Modu",
    "vietnamese": "Chế độ Big Picture trên Steam",
    "ukrainian": "Steam у режимі Big Picture",
}


def get_steam_language():
    try:
        key_path = r"Software\Valve\Steam\steamglobal"

        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, key_path) as key:
            language, reg_type = winreg.QueryValueEx(key, "Language")
            return language
    except FileNotFoundError:
        return None
    except Exception as e:
        print(f"An error occurred while accessing the registry: {e}")
        return None


def get_big_picture_window_title():
    language = get_steam_language()
    if language:
        return BIG_PICTURE_WINDOW_TITLES.get(language, BIG_PICTURE_WINDOW_TITLES["english"])
    else:
        return BIG_PICTURE_WINDOW_TITLES["english"]
