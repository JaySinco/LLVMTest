call plug#begin()
Plug 'https://hub.fastgit.xyz/scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
Plug 'https://hub.fastgit.xyz/vim-airline/vim-airline'
Plug 'https://hub.fastgit.xyz/easymotion/vim-easymotion'
Plug 'https://hub.fastgit.xyz/morhetz/gruvbox'
Plug 'https://hub.fastgit.xyz/tpope/vim-fugitive'
call plug#end()

set history=500
set t_Co=256
set number
set cursorline
let mapleader=","

syntax on
filetype plugin on
filetype indent on

set bg=dark
silent! colorscheme gruvbox

command! W execute 'w !sudo tee % > /dev/null' <bar> edit!

nnoremap <C-t> :NERDTreeToggle<CR>

let g:airline_section_b='%{getcwd()}'
let g:airline#extensions#tabline#enabled=1
let g:airline#extensions#tabline#buffer_idx_mode = 1

nmap <leader>1 <Plug>AirlineSelectTab1
nmap <leader>2 <Plug>AirlineSelectTab2
nmap <leader>3 <Plug>AirlineSelectTab3
nmap <leader>4 <Plug>AirlineSelectTab4
nmap <leader>5 <Plug>AirlineSelectTab5
nmap <leader>6 <Plug>AirlineSelectTab6
nmap <leader>7 <Plug>AirlineSelectTab7
nmap <leader>8 <Plug>AirlineSelectTab8
nmap <leader>9 <Plug>AirlineSelectTab9
nmap <leader>0 <Plug>AirlineSelectTab0
nmap <C-Left>  <Plug>AirlineSelectPrevTab
nmap <C-Right> <Plug>AirlineSelectNextTab
