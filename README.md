# Graph rendering tool

## Building instructions

To build simply perform on a terminal:

``` bash
    bash scripts/make.sh
    cd build/release
    make -j$(nproc)
```

## Execution instructions

To run simply:

```bash 
    ./build/release/detravisualizer
```

## Running instructions

Open the top pane menu items for selecting the different submenus.

Usual workflow you can generate the graphs using the Generate tool once is generated you can plot them using the layout tool which takes the last generated graph and plots it using the current parameters.

You can pan and rotate the camera.

Currently there is no way to load graphs but you can still generate them from a variety of generators.

All networks should be under the networks directory.
