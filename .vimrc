" echo set exrc > ~/.vim/vimrc
" curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.staticdn.net/junegunn/vim-plug/master/plug.vim

call plug#begin()
Plug 'https://hub.fastgit.xyz/scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
Plug 'https://hub.fastgit.xyz/vim-airline/vim-airline'
Plug 'https://hub.fastgit.xyz/easymotion/vim-easymotion'
Plug 'https://hub.fastgit.xyz/morhetz/gruvbox'
Plug 'https://hub.fastgit.xyz/airblade/vim-gitgutter'
call plug#end()

set history=500
set t_Co=256
set number
let mapleader=","

syntax on
filetype plugin on
filetype indent on

set bg=dark
colorscheme gruvbox

let g:gitgutter_sign_added = '+'
let g:gitgutter_sign_modified = '*'
let g:gitgutter_sign_removed = '-'
let g:gitgutter_sign_removed_first_line = '^'
let g:gitgutter_sign_removed_above_and_below = '{'
let g:gitgutter_sign_modified_removed = '~'

nnoremap <C-t> :NERDTreeToggle<CR>
command! W execute 'w !sudo tee % > /dev/null' <bar> edit!
