all: bcc-eval

bcc-eval: bcc-eval.c
	gcc bcc-eval.c -o bcc-eval

bcc-eval-d: bcc-eval.c
	gcc bcc-eval.c -g -DDEBUG -o bcc-eval
clean:
	rm -f *~ bcc-eval
