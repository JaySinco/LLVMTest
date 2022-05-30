" echo set exrc > ~/.vim/vimrc
" curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.staticdn.net/junegunn/vim-plug/master/plug.vim

call plug#begin()
Plug 'https://hub.fastgit.xyz/scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
call plug#end()

set number
nnoremap <C-t> :NERDTreeToggle<CR>
