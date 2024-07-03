# Kabegami ❖
**Kabegami** — это приложение для установки видео обоев на рабочий стол
- Простота использования

## Установка
### 1. Установка зависимостей
```sh
sudo apt-get update
sudo apt-get install build-essential cmake libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libx11-dev libxrandr-dev
```

### 2. Сборка
```sh
git clone https://github.com/ToshibaMastr/Kabegami
cd Kabegami
mkdir build && cd build
cmake .. && make
```

### 3 Установка в систему
```sh
sudo -E make install
```

### 4 Удаление из системы
```sh
sudo make uninstall
```

## Быстрый запуск

Запуск **Kabegami** с установкой обоев `video.mp4` и бесконечный повтор:

```sh
kabegami --loop video.mp4
```

## Документация

Для получения информации о доступныхопциях запуска используйте:

```sh
kabegami --help
```

## Лицензия
Этот проект лицензирован на условиях GNU General Public License v3.0. Подробности смотрите в файле [LICENSE](LICENSE).
