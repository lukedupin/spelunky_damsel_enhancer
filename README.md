# Spelunky Damsel Enhancer
Only for linux.  Gives you extra hearts for rescuing the damsel

# Compile and run

    qmake
    make
    sudo ./Spelunky [pid]

# Usage

By default, this will make saving a damsel worth 4 hearts.  You can provide the number of hearts you want to make a damsel worth: sudo ./Spelunky [pid] [number_of_hearts]

Also, if you provide a negative number, then the damsels are cumulative, meaning the heart boost increases by 1 for each damsel that is saved: sudo ./Spelunky [pid] -3

