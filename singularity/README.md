<!--
SPDX-FileCopyrightText: 2023 Christoph Sommer <sommer@ccs-labs.org>
SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>

SPDX-License-Identifier: GPL-2.0-or-later
-->

# singularity-space_veins
-----------------

Scripts for building a [Singularity][SYLABS] container for quickly building and running space_Veins simulations anywhere.


## Tested with ##

- [Singularity][SYLABS] 3.11.0
- [Debootstrap][DEBIAN] 1.0.126

[SYLABS]: https://sylabs.io/
[DEBIAN]: https://wiki.debian.org/Debootstrap


## Building ##

```
singularity build --fakeroot singularity-space_veins.sif singularity-space_veins.def
```

## Help ##

```
singularity run-help singularity-space_veins.sif
```

## Building/running simulations ##

```
singularity run -H ..:/space_veins --net --network none singularity-space_veins.sif --chdir /space_veins -- make makefiles
singularity run -H ..:/space_veins --net --network none singularity-space_veins.sif --chdir /space_veins -- make
singularity run -H ..:/space_veins --net --network none singularity-space_veins.sif --chdir /space_veins/examples/space_veins --launchd -- ./run -u Cmdenv -c Null-Island-Launchd -r 0
```

Alternatively the following commands can be used:

```
./singularity-space_veins.sh /space_veins -- make makefiles
./singularity-space_veins.sh /space_veins -- make
./singularity-space_veins.sh /space_veins/examples/space_veins --launchd -- ./run -u Cmdenv -c Null-Island-Launchd -r 0
```

## License ##

space_Veins is composed of many parts. See the version control log for a full list of
contributors and modifications. Each part is protected by its own, individual
copyright(s), but can be redistributed and/or modified under an open source
license. License terms are available at the top of each file. Parts that do not
explicitly include license text shall be assumed to be governed by the "GNU
General Public License" as published by the Free Software Foundation -- either
version 2 of the License, or (at your option) any later version
(SPDX-License-Id: GPL-2.0-or-later). Parts that are not source code and
do not include license text shall be assumed to allow the Creative Commons
"Attribution-ShareAlike 4.0 International License" as an additional option
(SPDX-License-Id: GPL-2.0-or-later OR CC-BY-SA-4.0). Full license texts
are available with the source distribution.

