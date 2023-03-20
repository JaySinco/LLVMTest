GIT_EXEC_PATH="$(git --exec-path 2>/dev/null)"
GIT_PROMPT_SH="$GIT_EXEC_PATH/git-sh-prompt"

if test -f $GIT_PROMPT_SH
then
	. $GIT_PROMPT_SH

	PS1='\[\033]0;\u@\h: \w\007\]' # set window title
	PS1="$PS1"'\n'                 # new line
	PS1="$PS1"'\[\033[32m\]'       # change to green
	PS1="$PS1"'\u@\h '             # user@host<space>
	PS1="$PS1"'\[\033[35m\]'       # change to purple
	PS1="$PS1"'$MSYSTEM '          # show MSYSTEM
	PS1="$PS1"'\[\033[33m\]'       # change to brownish yellow
	PS1="$PS1"'\w'                 # current working directory
	PS1="$PS1"'\[\033[36m\]'       # change color to cyan
	PS1="$PS1"'`__git_ps1`'        # bash function
	PS1="$PS1"'\[\033[0m\]'        # change color
	PS1="$PS1"'\n'                 # new line
	PS1="$PS1"'$ '                 # prompt: always $
fi