# None of these permutations work what the fuck is this hot trash
# !CMDSWITCHES NOLOGO /NOLOGO -NOLOGO

CC=cl
CFLAGS=/nologo /O1 /W4 /WX /wd4706 /wd4996
SRC=..
OUT=.

build: $(OUT)\GroundedLink.exe

$(OUT)\GroundedLink.exe: $(SRC)\GroundedLink.c
	$(CC) $(CFLAGS) \
	      -o $@ \
	      $(**) \
	      /link "user32.lib"

clean:
	del "$(OUT)\GroundedLink.exe" "$(OUT)\GroundedLink.obj"
