au FileType cpp,cc,cxx,h,hpp,hxx call <SID>KatatsumuriCompleteInit()

" autocmd FileType cpp,cc,cxx,h,hpp,hxx setlocal completefunc=KatatsumuriComplete
autocmd FileType cpp,cc,cxx,h,hpp,hxx setlocal omnifunc=KatatsumuriComplete

let s:katatsumuri_complete_list = []

function! s:KatatsumuriCompleteInit()
  if !exists('g:katatsumuri_complete_binary')
    let g:katatsumuri_complete_binary = 'katatsumuri_complete'
  endif

  if !exists('g:katatsumuri_complete_clang_binary')
    let g:katatsumuri_complete_clang_binary = 'clang'
  endif

  if !exists('g:katatsumuri_complete_opts')
    let g:katatsumuri_complete_opts = ''
  endif
endfunction

function! KatatsumuriComplete(findstart, base)
  if a:findstart
    let line = getline('.')
    let start = col('.') - 1
    while start > 0 && line[start - 1] =~ '\a'
      let start -= 1
    endwhile
    return start
  else
    let l:buffer = getline(1, '$')
    let l:tempfile = expand('%:p:h') . '/' . localtime() . expand('%:t')
    try
      call writefile(l:buffer, l:tempfile)
    catch /^Vim\%((\a\+)\)\=:E482/
      return []
    endtry
    let l:li = line('.')
    let l:co = col('.')
    let l:cm = g:katatsumuri_complete_binary.' '.g:katatsumuri_complete_clang_binary.' '.l:tempfile.' '.l:li.' '.l:co.' 1024 '.g:katatsumuri_complete_opts
    let l:clang_output = split(system(l:cm), "\n")
    call delete(l:tempfile)
    if len(l:clang_output) == 0
      return []
    endif
    let s:katatsumuri_complete_list = []
    for l:element in l:clang_output
      if strlen(l:element) > 13 && l:element[0:12] ==? '<katatsumuri>'
        let l:elementf = split(l:element[13:], '#')
        call add(s:katatsumuri_complete_list, { 'word': l:elementf[0], 'menu': l:elementf[1], 'dup': 1 })
      endif
    endfor
    return s:katatsumuri_complete_list
  endif
endfunction

