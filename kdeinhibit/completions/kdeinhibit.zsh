#compdef kde-inhibit

# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

local ret=1

_arguments -C \
  '(- *)'{-h,--help}'[Displays help on commandline options]' \
  '--power[Inhibit power management.]' \
  '--screenSaver[Inhibit screensaver.]' \
  '--colorCorrect[Inhibit colour correction (night mode).]' \
  '--notifications[Inhibit notifications (Do not disturb).]' \
  '--help-all[Displays help including Qt specific options..]' \
  '*::program: _command_names:*::program arguments: _normal' \
  && ret=0

return $ret
