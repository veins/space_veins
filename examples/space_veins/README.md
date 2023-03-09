<!--
SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>

SPDX-License-Identifier: GPL-2.0-or-later
-->

## space_Veins Example

This simulation requires to source the setenv file in the space_Veins root directory:
```
cd ../../
source setenv
cd examples/space_veins
```

If you want to use the configuration using veins_launchd, you have to start it first:
```
python3 ../../lib/veins/bin/veins_launchd -vv
```

Now you can run the example simulation:
```
./run -u Qtenv
```
