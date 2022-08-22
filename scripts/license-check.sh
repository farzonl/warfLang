#!/bin/bash

# sudo apt-get install golang or brew install golang
# go get -u github.com/google/addlicense


exit_if_error() {
  local exit_code=$1
  shift
  [[ $exit_code ]] &&               # do nothing if no error code passed
    ((exit_code != 0)) && {         # do nothing if error code is 0
      printf 'ERROR: %s\n' "$@" >&2 # we can use better logging here
      exit "$exit_code"             # we could also check to make sure
                                    # error code is numeric when passed
    }
}

~/go/bin/addlicense -check src
exit_if_error $? "license check failed in src."

~/go/bin/addlicense -check test
exit_if_error $? "license check failed in test."

~/go/bin/addlicense -check fuzz
exit_if_error $? "license check failed in fuzz."

echo "license check successfully exited!"
exit 0