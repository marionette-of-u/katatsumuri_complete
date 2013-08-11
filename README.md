**katatsumuri_complete**
=================

Todo & etc...
-----------

* kind 情報の追加.

Description
-----------

clang\_complete の設定が難しすぎるので自分で簡易的な補完スクリプトを書いてみた.   
namekuji\_complete とは違い libclang の組み込み binary ではなくコンパイラを直に呼び出している.   
katatsumuri\_complete.cpp をコンパイル, 実行できる環境 (C++11) と clang, vimproc, neocomplcache に依存.

Installation
-----------

* vimprocを入れる.
* plugin/katatsumuri_complete.vim を適切な場所に配置する.
* cpp/katatsumuri_complete.cpp をコンパイルし実行ファイルを生成, 好きな場所に配置する.
* .vimrc に後述する Setting examples に倣った設定を記述.

Setting examples
-----------

```vim
"---------------------------------------------------------------------------
" namekuji_complete

" katatsumuri_complete の実行形式ファイル.
let g:katatsumuri_complete_binary = 'C:/katatsumuri_complete/katatsumuri_complete.exe'

" clang の実行形式ファイル.
let g:katatsumuri_complete_clang_binary = 'C:/clang/clang.exe'

" 同名の補完結果でも signature ごとに分ける場合は 1, そうでない場合は 0.
let g:katatsumuri_complete_split_each_sig = 0

" その他オプション.
let g:katatsumuri_complete_opts = '-IC:/MinGW/msys/1.0/local/include -IC:/MinGW/lib/gcc/mingw32/4.7.2/include -IC:/MinGW/lib/gcc/mingw32/4.7.2/include/c++ -IC:/MinGW/lib/gcc/mingw32/4.7.2/include/c++/mingw32'


"---------------------------------------------------------------------------
" neocomplcache
" 良しなに.

let g:neocomplcache_enable_at_startup = 1
let g:neocomplcache_max_list = 2048

if !exists('g:neocomplcache_filename_include_exts')
  let g:neocomplcache_filename_include_exts = {}
endif
let g:neocomplcache_filename_include_exts.cpp = ['', 'h', 'hpp', 'hxx']

if !exists('g:neocomplcache_omni_patterns')
  let g:neocomplcache_omni_patterns = {}
endif
let g:katatsumuri_omni_pattern = '[^.[:digit:] *\t]\%(\.\|->\)\|\h\w*::'
let g:neocomplcache_omni_patterns.cpp = g:katatsumuri_omni_pattern
let g:neocomplcache_omni_patterns.cc = g:katatsumuri_omni_pattern
let g:neocomplcache_omni_patterns.cxx = g:katatsumuri_omni_pattern
let g:neocomplcache_omni_patterns.h = g:katatsumuri_omni_pattern
let g:neocomplcache_omni_patterns.hpp = g:katatsumuri_omni_pattern
let g:neocomplcache_omni_patterns.hxx = g:katatsumuri_omni_pattern
```
