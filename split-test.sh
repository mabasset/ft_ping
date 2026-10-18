#!/bin/bash

SESSION=pingcmp

script=$(realpath "test.sh")
tmux kill-session -t "$SESSION" 2>/dev/null
left=$(tmux new-session -d -s "$SESSION" -P -F '#{pane_id}')
right=$(tmux split-window -h -t "$left" -P -F '#{pane_id}')

tmux send-keys -t "$left" "clear; bash '$script' ft_ping L R" Enter
tmux send-keys -t "$right" "clear; bash '$script' ping R L" Enter
tmux set-window-option -t "$SESSION" synchronize-panes on
tmux set-option -t "$SESSION" mouse on
tmux attach -t "$SESSION"
