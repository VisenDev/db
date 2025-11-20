# Business Database Management application written in C

## To Build

```
cc build.c -o build && ./build
```

## To Cross Compile

### From a 'nix to windows
```
cc build.c -o build
CC="zig cc --target=x86_64-windows" TARGET=windows ./build
```