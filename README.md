# libflow

What if each node / flow was a dynamically loaded shared library? And a host program just loaded everything initially and then just ran as it pleased?

## Running
```sh
pnpm i
make
./flowd
```

On another terminal:
```sh
telnet localhost 4201
LOAD flow1
RUN 0
LOAD pyfun
RUN 1
```

## Advantages
- Separation of flow / node initialization and execution. This allows for needed connections to be established only once per node type.
- Python imports resolved on load
- Python only initialized once
- Nodes or flows can be written as shared libraries, which allows for a variety of languages.
- Should be thread safe by default
- WS(S) / dynamic HTTP(S) support for updates is built-in by default. WS can notify clients with state updates for node lights to light up, for example.

## Documentation
You should have language server information about all the functions you need.
```sh
make
man ./docs/man/man3/flow.h.3
```

## Structure
As I said, each node and flow / subflow is a dynamically loaded share library. They link a user-api library so that a host program can provide functions for them to call.

Then the host program just runs the initial flow.

## Awknowledgement
I have the pleasure to announce this project was open-sourced with the permission of mov.ai. That way I'll be free to work on it on my free time to make a visual programming language for my game, since in mov.ai I have FE tasks to attend to.

Thank you very much 🙏.
I'll do my best with what I've been given.
