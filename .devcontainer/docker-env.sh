# sourced for every new shell (by placing in /etc/profile.d or similar)

#
# Copyright (C) 2023 Christoph Sommer <sommer@cms-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# The following list shows changes made by Mario Franke
# Copyright (C) 2023 Mario Franke <research@m-franke.net>
# Documentation for space_Veins is at http://sat.car2x.org/
# SPDX-License-Identifier: GPL-2.0-or-later
#
# 2023-08-17 change PYTHONPATH variable because Ubuntu Jammy brings
#            python3.10
#            add /opt/mambaforge/bin to PATH variable

export PYTHONPATH=/opt/pip/usr/local/lib/python3.10/dist-packages:$PYTHONPATH
export PATH=/usr/lib/ccache:$PATH
export PATH=$PATH:/opt/omnetpp/bin
export PATH=$PATH:/opt/sumo/bin
export PATH=$PATH:/opt/mambaforge/bin
