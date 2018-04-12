### What have been modified
- The definition of `resuGraphe.dat`, the attribute `distance` and `inS` is added to each line.

### Features
- Add two new mode of views of result.
  - Show all edges and its weight mode
    To enable this mode, add `#define SHOW_ALL` to `prim.h`
  - Show every step mode
    In this mode, data will be saved in every iteration. All of this data will be saved in a folder `./steps`
    To enable this mode, add `#define SHOW_EVERY_STEPS` to `prim.h`
- Visualize these two new mode by plotlib
  - Show the weight of each edge
  - identify automatically which mode is used to create the data.

### How to use visualization
- Using command:
  - visualize in the `SHOW_EVERY_STEPS` mode
    ```
    python visualisation.py ./steps/ ./resuVilles.dat 0
    ```
    Considering the performance, picture display function is disabled. However, all of the pictures created are saved in the folder `./step_anim`

  - visualize in the other mode
    ```
    python visualisation.py ./resuGraphe.dat ./resuVilles.dat 0
    ```
  - Show weight
    ```
    python visualisation.py ./steps/ ./resuVilles.dat 1
    ```
    or
    ```
    python visualisation.py ./resuGraphe.dat ./resuVilles.dat 1 
    ```
