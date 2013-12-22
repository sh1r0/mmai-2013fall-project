DIR=.
find $DIR -type f -name '*_p.jpg' | while read f; do
	./extract_roi $f
done
