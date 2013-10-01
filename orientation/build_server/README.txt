The platform folders contains build scripts used by the Jenkins 
build servers.

There are separate scripts for each build step.
Each step script does only that step.
It will rely on previous steps having succeeded.
They are not intended for human use.

There is an environment setup script (environ.sh)
that is referenced from the build step scripts but does
no actual processing.

the common folder contains scripts that may be sourced
or executed by the various platform scripts.

