[src/sample_app.cc](https://github.com/dreamer-dead/chromium-sample-app/blob/470736619faaba62561d552b16665fdb69a0fe5a/src/sample_app.cc)
```c++
int main() {
  return 0;
}
```
[src/BUILD.gn](https://github.com/dreamer-dead/chromium-sample-app/blob/8248c8550ea99887b555776c1c4a4e1da91259bd/src/BUILD.gn)
```c++
# SampleApp

executable("sample_app") {
  output_name = "sample_app"
  sources = [
    "sample_app.cc",
  ]
}
[src/root_BUILD_gn.patch](https://github.com/dreamer-dead/chromium-sample-app/blob/d346775d3392a9082c53b45547f53903e69dd5bd/src/root_BUILD_gn.patch)