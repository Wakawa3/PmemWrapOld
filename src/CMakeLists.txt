add_library(pmemwrap
  SHARED
    pmemopenwrap.c
  )
# greetingライブラリのインクルードディレクトリを教えてあげる
# PROJECT_SOURCE_DIRはこのプロジェクトのルートディレクトリの絶対パス
target_include_directories(pmemwrap
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  )