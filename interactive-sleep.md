---
layout: page
category: b
tags: [ shell, sleep ]
date: 2023-08-24
status: finished
title: Interactive sleep
---

The `sleep` utility is something I'd hardly consider worth enhancing by
features, the task is simple to sleep for a given time. Nothing else is needed
when it's used from scripts. However, it can be also used from manually typed
command line, e.g. to let some command start later to offset initial IO load or
not to overload network.

Let's say for example that we'd like to download a big data set, but rather let
it start in 20 minutes.

```
sleep 20m && wget https://URL
```

Once this command starts we don't know how much time is remaining and
interrupting it by `Ctrl-C` would not start the download. Additionally, what if
the time is too short and we'd like to increase it. So let's implement it. For
more compatibility it's done in shell and using only widely available utilities
(`date`). And btw, it does not even use `sleep` for the sleeping but the builtin
`read` with the timeout option as it allows to read input while waiting.

<details open="true">
<summary>sleepy the shell script</summary>
<pre>
#!/bin/sh

t=${1:-1}
ts="$t"

case "$t" in
	*s) tm=${t%*s} t=$(($tm)) ;;
	*m) tm=${t%*m} t=$(($tm * 60)) ;;
	*h) tm=${t%*h} t=$(($tm * 60 * 60)) ;;
	*d) tm=${t%*d} t=$(($tm * 60 * 60 * 24)) ;;
	*[0-9]) : ;;
	*) echo "ERROR: unknown timestamp"; exit 1;;
esac

while :; do
	[ "$t" -le 0 ] && break
	read -n 1 -t 1 ch
	ret=$?
	if [ $ret = 142 ]; then
		# timeout
		:
	elif [ $ret = 0 ]; then
		# keypress
		echo "sleep for $ts, remaining" `TZ=utc date --date=@$t +%H:%M:%S` "($t)"
		if [ "$ch" = 'q' ]; then
			echo "quit"
			exit 0
		elif [ "$ch" = '+' ]; then
			echo "increase by 10s"
			t=$(($t + 10 - 1))
		elif [ "$ch" = '-' ]; then
			echo "decrease by 10s"
			t=$(($t - 10 - 1))
		fi
		t=$(($t + 1))
	fi
	t=$(($t - 1))
done
</pre>
</details>

The time passed on command line needs to be parsed and it does not support
fractional times, otherwise it's straightforward. The interesting part is
that it reacts to key presses:

* `q` stop waiting, with exit code 0 so the next command could continue,
  otherwise `Ctrl-C`
* `+` increase waiting time by 10 seconds and print it
* `-` decrease waiting time by 10 seconds and print it
* anything else prints remaining time (and slightly increases the
  waiting time due to processing)
