Я же хотел рассказать о том, как на основе этого кода можно создавать приложения на C++,
Не стану останавливаться на том, как скачать код и настроить окружение, все это подробно описано в статьях по приведеным ссылкам. Будем считать, что у нас уже есть depot_tools в нашем $PATH (нужны утилиты gn и ninja), исходный код получен и готов к сборке в директории chromium/. Сборка всего проекта Chromium нам не понадобится, по крайней мере на первом этапе.

Создадим в chromium/src директории нашего приложения, sample_app и sample_app/src.
В sample_app/src будет размещен код приложения, а все команды я буду приводить относительно текущей директории chromium/src/sample_app.

Чтобы получить сразу весь код приложения из статьи, можно склонировать репозиторий
https://github.com/dreamer-dead/chromium-sample-app.git

```bash
$ pwd
/Users/username/chromium/src

$ git clone https://github.com/dreamer-dead/chromium-sample-app.git sample_app

$ cd sample_app/
```

diff --git a/src/sample_app.cc b/src/sample_app.cc
+++ b/src/sample_app.cc
diff --git a/src/BUILD.gn b/src/BUILD.gn
index 0000000..b2fbea5
+++ b/src/BUILD.gn
@@ -0,0 +1,8 @@
более дружелюбен для человека. Так что будем использовать именно его, хотя на 
В своем билд-конфиге sample_app/src/BUILD.gn задаем имя таргета, итоговое имя исполняемого файла и 
Для того, чтобы GN увидел конфиг нашего проекта, нужно сослаться на него в корневом файле 
chromium/src/BUILD.gn, наложив такой патч
```diff
diff --git a/BUILD.gn b/BUILD.gn
index ce698a7..67f8219 100644
--- a/BUILD.gn
+++ b/BUILD.gn
@@ -886,3 +886,7 @@ group("chromium_builder_perf") {
     }
   }
 }
+
+group("sample_app") {
+  deps = [ "//sample_app/src:sample_app" ]
+}
```

Если был сделан чекаут репозитория, то для этого можно выполнить
```bash
$ (cd .. && git apply sample_app/src/root_BUILD_gn.patch)
```

$ gn gen --args=is_debug=true --root=../ ../out/gn
$ ninja -C ../out/gn sample_app
Таким образом, мы указали, что собираем отладочную сборку, генерируем ninja-файлы сборки в 
директории chromium/src/out/gn/ и корневой билд-конфиг располагается в chromium/src/

Давайте добавим консольный вывод в наше приложение и покажем, что как минимум нам доступна стандартная библиотека С++.
diff --git a/src/sample_app.cc b/src/sample_app.cc
--- a/src/sample_app.cc
+++ b/src/sample_app.cc
$ ninja -C ../out/gn sample_app
$ ../out/gn/sample_app