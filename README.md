Usage:
  Start msi-keyboard-manager as root.
	You can manipulate your keyboard with the pipe file at /var/run/msi-keyboard-manager/cmd
	You can put any recognized command into '/etc/msi-keyboad-manager/start.conf' to load values when the programm starts.

Cmd:
	Change keyboard brightness: 
		echo "brightness (disabled|low|mid|high)" >>/var/run/msi-keyboard-manager/cmd
	Example: (disable keyboard leds)
		echo "brightness disabled" >>/var/run/msi-keyboard-manager/cmd

	Change keyboard mode: color format: #RRGGBB
		echo "normal_mode (left|middle|right) (color)" >>/var/run/msi-keyboard-manager/cmd
		echo "wave_mode (left|middle|right) (start color) (end color) (speed) >>/var/run/msi-keyboard-manager/cmd
		echo "breathing_mode (left|middle|right) (start color) (end color) (speed) >>/var/run/msi-keyboard-manager/cmd

	Example: (blue left area)
		echo "normal_mode left #0000FF" >>/var/run/msi-keyboard-manager/cmd
