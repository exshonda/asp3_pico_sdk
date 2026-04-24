# TOPPERS/ASP3×pico-sdk

[TOPPERS/ASP3](https://github.com/exshonda/asp3_pico_sdk)を[pico-sdk](https://github.com/raspberrypi/pico-sdk)と使えるようにしたプロジェクトの例です。

[Visual Studio Code](https://code.visualstudio.com/)の[Raspberry Pi Pico 拡張機能](https://github.com/raspberrypi/pico-vscode)で作成したプロジェクトに、`asp3`を追加することで、シングルコア対応 RTOS API が使えます。

Raspberry Pi Pico 2（RP2350、ARM Cortex-M33）で使えます。

## リポジトリのクローン

`asp3`フォルダはサブモジュールとなっているので、下記のコマンドでサブモジュールもクローンしてください。

```bash
git clone https://github.com/exshonda/asp3_pico_sdk_sample.git
cd asp3_pico_sdk_sample
git submodule update --init
```

## ビルド方法

Pico 拡張機能のビルド機能をそのまま使用します。

![Pico 拡張機能](images/pico_extention_compile.png)

## 新規プロジェクトへの適用方法

Pico 拡張機能で作成したプロジェクトに TOPPERS/ASP3 を追加することが出来ます。その手順を説明します。

[ASP3のリポジトリ](https://github.com/exshonda/asp3_pico_sdk)を、`git clone`などで適当な場所に配置します。

Pico 拡張機能でプロジェクトを作成し、そのプロジェクトの`CMakeLists.txt`を編集して、TOPPERS/ASP3 を配置したフォルダをビルド対象に含めるようにします。

サンプルアプリの`CMakeLists.txt`を見て編集箇所を確認していください。

<https://github.com/exshonda/asp3_pico_sdk_sample/blob/main/sample1/CMakeLists.txt>

まず、`asp3_pico_sdk.cmake`ファイルのインクルードを行うため、下記の行を追加します。

- プロジェクトフォルダと同じ階層に`asp3`フォルダを配置した場合

    ```cmake
    # TOPPERS/ASP3 の Raspberry Pi Pico SDK 用のコンパイラ定義をインクルード
    include(../asp3/asp3_pico_sdk.cmake)
    ```

    このリポジトリはこの配置です。

- 別のフォルダに置いた場合

    下記の`path/to/asp3`を書き換えて、`asp3`を配置したフォルダの絶対パスか、相対パスで書き換えます。

    ```cmake
    # TOPPERS/ASP3 の Raspberry Pi Pico SDK 用のコンパイラ定義をインクルード
    include(path/to/asp3/asp3_pico_sdk.cmake)
    ```

タスクやセマフォなどを静的APIで定義したcfgファイルを指定します。
このリポジトリでは[asp3/sample/sample1.cfg](asp3/sample/sample1.cfg)となっていますが、他の名前に変更できます。

```cmake
# TOPPERS/ASP3 のカーネルオブジェクト定義のcfgファイルを設定
set(ASP3_APP_CFG_FILE ${ASP3_CMAKE_DIR}/sample/sample1.cfg)
```

asp3をサブプロジェクトとして追加します。
上記の`include`で、ASP3へのパスが`ASP3_CMAKE_DIR`という変数に設定されるので、以降で使用できます。

```cmake
# TOPPERS/ASP3 のライブラリを追加
add_subdirectory(${ASP3_CMAKE_DIR} asp3)
```

`add_subdirectory`の第２引数は、ビルド時に出力フォルダ名として使用されます。

TOPPERS/ASP3 付属のソースファイルを追加します。このサンプルのcfgファイルでは必要ですが、cfgファイルの内容に応じて編集してください。

```cmake
# TOPPERS/ASP3 付属のソースファイルを追加
include(${ASP3_CMAKE_DIR}/library/library.cmake)
include(${ASP3_CMAKE_DIR}/syssvc/syssvc.cmake)
```

TOPPERS/ASP3 ライブラリの`asp3`を追加します。

```cmake
# Add the standard library to the build
target_link_libraries(sample1_pico_sdk
    pico_stdlib
    asp3
)
```

最後に「`asp3_set_pico_sdk_options`」関数を呼び出します。引数にはプロジェクト作成時に付けた名前を設定します。
プロジェクト名は`CMAKE_PROJECT_NAME`変数で取得できます。

```cmake
# TOPPERS/ASP3 を使うための Raspberry Pi Pico SDK の設定
asp3_set_pico_sdk_options(${CMAKE_PROJECT_NAME})
```

TOPPERS/ASP3 で使用する変数が想定通りのメモリ配置になっているかチェックが行えますが、pico-sdk では使用していないシンボルを削除するリンクオプションが付いてているので、チェックは行えないため省略します。

```cmake
# TOPPERS/ASP3 のチェックを行う（シンボルがGCされるとエラーになるので省略）
#asp3_cfg_check(${CMAKE_PROJECT_NAME})
```
