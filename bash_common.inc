#!/bin/bash

#
#	Name : bash_common.inc
#	Role : Fonctions partagées pour les scripts bash
#	Creator : Julien Groselle
#	Date : 28/11/2012
#

function debug() {
	echo "[DEBUG] (${FUNCNAME[1]}:${BASH_LINENO[0]}) $*"
}

function f_notify () {
	# Déclaration des variables statiques
	local MY_DATE="$(date '+%Y-%m-%d_%H:%M:%S')"
	local MY_DEBUG=0
	MY_MESSAGE=""
	MY_TARGET_SCREEN=0
	MY_TARGET_FILE=0
	MY_TARGET_X=0
	MY_EXIT_CODE=0
	MY_EXIT=""

	# OPTIND garde en mémoire les options passées à un autre getopts, il faut donc le décharger
	unset OPTIND

	# Déclaration des variables dynamiques

	# Déclaration des fonctions
	notify_usage() {
		script_name=$(basename $0)
		echo ""
		echo "  ${FUNCNAME[1]} usage:"
		echo ""
		echo "  ${FUNCNAME[1]} -m \"Message\" -s -c 0 -e n"
		echo ""
		echo "  ${FUNCNAME[1]} -h : Affiche cette aide"
		echo "    Gestion de la sortie :"
		echo "      -s : Sortie standard"
		echo "      -f : Sortie fichier"
		echo "      -x : Sortie par info-bulle"
		echo ""
		echo "      -c : Code d'erreur [0|1|2|3]"
		echo "      -e : Sortie du script [y|n]"
		echo "      -m : Message a afficher"
		echo ""
		exit 3
	}

	while getopts ":hdsfexm:c:" notify_opt; do
		case $notify_opt in
			h) notify_usage ;;
			d) MY_DEBUG=1 ;;
			m)
				(( $MY_DEBUG )) && debug -m was trigered
				MY_MESSAGE="$OPTARG"
			;;
			s)
				(( $MY_DEBUG )) && debug -s was trigered
				MY_TARGET_SCREEN=1
			;;
			f)
				(( $MY_DEBUG )) && debug -f was trigered
				MY_TARGET_FILE=1
			;;
			x)
				(( $MY_DEBUG )) && debug -x was trigered
				MY_TARGET_X=1
			;;
			c)
				(( $MY_DEBUG )) && debug -c was trigered
				MY_EXIT_CODE="$OPTARG"
			;;
			e)
				(( $MY_DEBUG )) && debug -e was trigered
				MY_EXIT=1
			;;
			:) echo "[ERROR] Option -$OPTARG requires an argument." && notify_usage ;;
			\?) echo "[ERROR] Invalid option -$OPTARG" && notify_usage ;;
		esac
	done

	# Affichage des variables :
	(( $MY_DEBUG )) && echo ""
	(( $MY_DEBUG )) && echo "Affichage des variables"
	(( $MY_DEBUG )) && echo "\$MY_DEBUG = $MY_DEBUG"
	(( $MY_DEBUG )) && echo "\$MY_MESSAGE = $MY_MESSAGE"
	(( $MY_DEBUG )) && echo "\$MY_TARGET_SCREEN = $MY_TARGET_SCREEN"
	(( $MY_DEBUG )) && echo "\$MY_TARGET_FILE = $MY_TARGET_FILE"
	(( $MY_DEBUG )) && echo "\$MY_TARGET_X = $MY_TARGET_X"
	(( $MY_DEBUG )) && echo "\$MY_EXIT_CODE = $MY_EXIT_CODE"
	(( $MY_DEBUG )) && echo "\$MY_EXIT = $MY_EXIT"
	(( $MY_DEBUG )) && echo ""

	# Gestion du log sur la sortie standard
	(( $MY_TARGET_SCREEN )) && {
		([ "$MY_EXIT_CODE" = "0" ] && echo "$MY_DATE [INFO] ${MY_MESSAGE}") ||
		([ "$MY_EXIT_CODE" = "1" ] && echo "$MY_DATE [WARN] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE") ||
		([ "$MY_EXIT_CODE" = "2" ] && echo "$MY_DATE [CRIT] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE") || echo "[UNKN] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE" ; }

	# Gestion du log sur la sortie texte
	((	$MY_TARGET_FILE )) && {
		[[ $MY_LOGFILE ]] || echo "Attention, \"f_notify\" a besoin d'une variable MY_LOGFILE déclarée"
		[ -e "$MY_LOGFILE" ] || (touch "$MY_LOGFILE" &> /dev/null || echo "Impossible de créer le fichier $MY_LOGFILE") 
		[ -f "$MY_LOGFILE" ] && (
			([ "$MY_EXIT_CODE" = "0" ] && echo "$MY_DATE [INFO] $MY_MESSAGE" >> "$MY_LOGFILE") ||
			([ "$MY_EXIT_CODE" = "1" ] && echo "$MY_DATE [WARN] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE" >> "$MY_LOGFILE") ||
			([ "$MY_EXIT_CODE" = "2" ] && echo "$MY_DATE [CRIT] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE" >> "$MY_LOGFILE") || echo "$MY_DATE [UNKN] (${FUNCNAME[1]}:$BASH_LINENO) $MY_MESSAGE"  >> "$MY_LOGFILE" 
		) ; }

	# Gestion du log en popup (requiers un serveur X)
	(( $MY_TARGET_X )) && {
		# Récupération des informations des sessions X pour notification popup
		local MY_PIDS="$(pgrep 'gnome-session|kde-session|xfce4-session')"
		[ -z $MY_PIDS ] && echo "Pas de session graphique sur ce poste/serveur"

		for my_pid in $MY_PIDS ; do
			# Trouve les infos sur la session DBUS
			local MY_DBUS_S=`\grep -z DBUS_SESSION_BUS_ADDRESS /proc/${my_pid}/environ | \sed -e 's/DBUS_SESSION_BUS_ADDRESS=//'`
			local MY_UNAME=`\grep -z USERNAME /proc/${my_pid}/environ | \sed -e 's/USERNAME=//'`
			local MY_DISP=`\grep -z DISPLAY /proc/${my_pid}/environ | \sed -e 's/DISPLAY=//'`
			local MY_XAUTH=`\grep -z XAUTHORITY /proc/${my_pid}/environ | \sed -e 's/XAUTHORITY=//'`
			export DISPLAY=$MY_DISP DBUS_SESSIOMY_BUS_ADDRESS=$MY_DBUS_S XAUTHORITY=$MY_XAUTH

			([ "$MY_EXIT_CODE" = "0" ] && su $MY_UNAME -c "notify-send -u low '[INFO]' \"$MY_MESSAGE\"") ||
			([ "$MY_EXIT_CODE" = "1" ] && su $MY_UNAME -c "notify-send -u critical '[WARN]' \"$MY_MESSAGE\"") ||
			([ "$MY_EXIT_CODE" = "2" ] && su $MY_UNAME -c "notify-send -u critical '[CRIT]' \"$MY_MESSAGE\"") || su $MY_UNAME -c "notify-send -u low '[UNKN]' \"$MY_MESSAGE\""
		done ; }

	# OPTIND garde en mémoire les options passé a getopts, il faut donc le décharger
	unset OPTIND

	# Gestion de la sortie du script
	{ [[ "$MY_EXIT" ]] && exit $MY_EXIT_CODE ; } || return $MY_EXIT_CODE
}
