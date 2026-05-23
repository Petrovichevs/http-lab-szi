# Клиент-серверное HTTP-приложение на C с механизмами СЗИ

Данный проект реализует простое клиент-серверное HTTP-приложение на языке программирования C с обязательными механизмами **Системы Защиты Информации (СЗИ)**.

## Основные механизмы безопасности (СЗИ)

- **Whitelist** (белый список) — доступ разрешён только для указанных IP-адресов (файл `whitelist.conf`)
- **Аутентификация** — проверка секретного токена в заголовке `X-Auth`
- **Маркировка информации** (метки безопасности) — параметр `?label=0` или `?label=1`
- **Конфигурация сервера** с учётом требований СЗИ (`server.conf`)

## Этапы разработки

1. **Этап планирования и инициализации репозитория**  
   Создание структуры проекта, `.gitignore`, начального README. Определение требований.  
   *Коммит: Initial repo setup - этап планирования*

2. **Этап добавления конфигурационных файлов**  
   Создание `server.conf` и `whitelist.conf`. Реализация функций чтения конфигов.  
   *Коммит: Added configuration files (СЗИ)*

3. **Этап реализации основного функционала**  
   Полный код сервера (`server.c`) и клиента (`client.c`) с реализацией всех механизмов СЗИ.  
   *Коммит: Полный код сервера и клиента + СЗИ (whitelist, auth, метки)*

4. **Этап автоматизации сборки**  
   Создание и доработка `Makefile`.  
   *Коммит: Скрипты сборки для CI/CD*

5. **Этап финальной интеграции**  
   Доработка универсального `Makefile`.  
   *Коммит: Updated Makefile: universal version for server and client*

6. **Этап отладки и исправления**  
   Исправление ошибок компиляции и мелких багов.  
   *Коммит: Fixed compiler*

7. **Финальный коммит**  
   *Коммит: Final version: complete client-server HTTP app with SZI mechanisms*

## Основные возможности

### Как работает сервер (`server.c`)

1. **Запуск сервера**
   - Читает конфигурацию из файлов `server.conf` и `whitelist.conf`.
   - Переходит в режим демона (отсоединяется от терминала, работает в фоне).
   - Открывает TCP-сокет и начинает слушать указанный порт (по умолчанию 8080).

2. **Обработка подключений**
   - При новом соединении сервер создаёт дочерний процесс через `fork()`.
   - Дочерний процесс выполняет все проверки и отправляет ответ, после чего завершается.

3. **Механизмы защиты (СЗИ)** — проверяются строго по порядку:
   - **Whitelist**: IP-адрес клиента должен быть указан в `whitelist.conf`. Иначе → **403 Forbidden**.
   - **Аутентификация**: Обязателен заголовок `X-Auth:` с секретным токеном из `server.conf` (по умолчанию `mysecret123`). Иначе → **401 Unauthorized**.
   - **Метка безопасности (`label`)**: Параметр `?label=` в URL. Допустимы только значения `0` или `1`. Иначе → **400 Bad Request**.

4. **Ответ сервера**
   - `label=0` → обычная страница «Метка 0: Простая страница index.html».
   - `label=1` → «защищённые данные» (другая HTML-страница).
   - Все успешные ответы возвращаются с кодом **200 OK** в виде простого HTML.

### Как работает клиент (`client.c`)

Клиент — это консольная утилита, которая:
- Подключается по TCP к серверу.
- Формирует HTTP-запрос следующего вида:
```http
GET /index.html?label=0 HTTP/1.1
Host: ...
X-Auth: mysecret123
Connection: close
```
- Отправляет запрос серверу.
- Получает полный ответ от сервера и выводит его в консоль.

### Запуск клиента

**Клиент** отправляет запрос серверу, получает полный ответ и выводит его в консоль.

### Команда запуска

```bash
./client <IP_сервера> <порт> <метка_0_или_1> <секрет>
```
### Пример использования
 ```bash
./client 127.0.0.1 8080 1 mysecret123
```

```bash
./client 127.0.0.1 8080 0 mysecret123
```

## Скрипты сборки (Makefile)

### Команды сборки

- `make` — собрать клиента
- `make full` — собрать только сервер

### Запуск

- `make run-server` — запустить сервер
- `./server` — запуск сервера вручную (становится демоном)

### Очистка

- `make clean` — очистить скомпилированные файлы

## Диаграммы программы

### Архитектура системы (Component Diagram):
<img width="974" height="809" alt="image" src="https://github.com/user-attachments/assets/9c3408a0-90a4-4b97-979e-49a82fcdcb93" />

### Последовательность работы (Sequence Diagram):
<img width="974" height="702" alt="image" src="https://github.com/user-attachments/assets/454a76c4-3d63-4321-8a9b-f08dbc339808" />


# Тестирование лабораторной

## Запуск:

**Сборка на сервере:**
<img width="974" height="104" alt="image" src="https://github.com/user-attachments/assets/4f55c612-c666-46c7-be99-58ead3c6a1a9" />
**Сборка на клиенте:**
<img width="974" height="76" alt="image" src="https://github.com/user-attachments/assets/325ec6dd-7fa4-40ac-b9ce-a4b7321e61d4" />
**Запуск сервера:**
<img width="950" height="122" alt="image" src="https://github.com/user-attachments/assets/bb421382-e2f5-40e1-bab5-d1e39aeffd3c" />

## Тест 1: Нормальная работа (основной сценарий):

**На сервере:**
<img width="967" height="158" alt="image" src="https://github.com/user-attachments/assets/22b41801-c47a-448f-a603-59020d04ec23" />
**На клиенте с 0:**
<img width="974" height="183" alt="image" src="https://github.com/user-attachments/assets/6f4d7fe4-09c8-49ed-a310-ecb178671114" />
**На клиенте с 1:**
<img width="974" height="159" alt="image" src="https://github.com/user-attachments/assets/eb5ed09d-d77f-451d-9f20-196986dd0e25" />

## Тест 2: Проверка аутентификации (СЗИ):

**Не правильный пароль:**
<img width="974" height="151" alt="image" src="https://github.com/user-attachments/assets/7ad6932a-b4a9-45f2-a3e7-5c2b97fd3abf" />

## Тест 3: Проверка whitelist (самый важный тест СЗИ):

**Изменение на сервере whitelist:**
<img width="974" height="159" alt="image" src="https://github.com/user-attachments/assets/4c448ce6-6d89-4af3-96b7-fe33ffc9deec" />
**Перезапуск сервера:**
<img width="974" height="124" alt="image" src="https://github.com/user-attachments/assets/5c0e4ea4-5d81-4da7-9eb4-4ef9fb4c0df4" />
**На клиенте запуск с ip не из белого списка:**
<img width="974" height="202" alt="image" src="https://github.com/user-attachments/assets/8b49abae-e057-4e68-9cea-a5c6cfebd961" />

## Тест 4: Неправильная метка:

**Возвращение в whitelist на место:**
<img width="974" height="71" alt="image" src="https://github.com/user-attachments/assets/f34f8964-944c-4d2a-9dd3-b5a68dc430b3" />
**На клиенте с неправильной меткой:**
<img width="974" height="185" alt="image" src="https://github.com/user-attachments/assets/5138cc7c-5f37-4b21-85dd-984c1b413b68" />

## Тест 5: Правильное завершение процессов:

**Просмотр процессов на сервере:**
<img width="974" height="335" alt="image" src="https://github.com/user-attachments/assets/77860d43-b452-4bc8-ad4f-3595f0f67340" />
**Завершение**
<img width="974" height="78" alt="image" src="https://github.com/user-attachments/assets/7fb78e0c-5487-4d7b-b1e1-3b4d53b8ec7e" />
Все процессы сервера (и главный, и дочерние) полностью исчезли.\

## Тест 6: Сборка через make (CI/CD):

<img width="770" height="88" alt="image" src="https://github.com/user-attachments/assets/009331e1-224a-48f7-9adb-bf4acb692eca" />


# Вывод:
**Было реализовано:**
1.	Создан Git-репозиторий и организована работа с версиями кода. 
2.	Разработан HTTP-сервер (server.c):
•	Работает как демон;
•	Читает конфигурационные файлы при запуске;
•	Проверяет IP клиента по whitelist;
•	Обрабатывает каждый запрос в отдельном процессе (fork);
•	При получении сигнала SIGTERM/SIGINT сначала завершает все дочерние процессы, затем завершает работу сам.
3. Разработан клиент (client.c), который отправляет HTTP-запросы с меткой и секретным ключом. 
4. Создан файл Makefile для сборки клиента и сервера. 
