set nobackup
set nowritebackup
set noswapfile
set nocompatible
set incsearch
set scrolloff=2
set backspace=indent,eol,start
set autoindent
set copyindent
set number
set showmatch
set ignorecase
set smartcase
set smarttab
set history=1000
set undolevels=1000
set title
set nowrap
set tabstop=4
set noexpandtab
"if the file ends in make we use hard tabs
autocmd FileType make set noexpandtab

"color highlighting
hi Comment ctermfg=gray
hi Constant ctermfg=red
hi Identifier ctermfg=white
hi PreProc ctermfg=blue
hi Type ctermfg=yellow
hi Special ctermfg=DarkRed


imap <C-d> <esc>:w
imap <C-c> x
imap <C-x> x
imap <C-v> <esc>Pi

filetype plugin indent on "to use use 'autocmd filetype pythin set expandtab'

function Tab_Or_Complete()
    if col('.') > 1 && strpart(getline('.'), col('.')-2, 3) =~ '^\w'
        return "\<C-N>"
    else
        return "\<Tab>"
    endif
endfunction

inoremap <Tab> <C-R>=Tab_Or_Complete()<CR>
set dictionary="/usr/dict/words"
