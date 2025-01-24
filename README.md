# libflow

What if each node / flow was a dynamically loaded shared library? And a host program just loaded everything initially and then just ran as it pleased?

## Running
```sh
pnpm i
make
./mov flow1
```

## Structure
As I said, each node and flow / subflow is a dynamically loaded share library. They link a user-api library so that a host program can provide functions for them to call.

Then the host program just runs the initial flow.
