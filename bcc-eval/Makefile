all: bcc-eval-d

bcc-eval: bcc-eval.c
	gcc bcc-eval.c -o bcc-eval

bcc-eval-d: bcc-eval.c
	gcc bcc-eval.c -DDEBUG -o bcc-eval
clean:
	rm -f *~ bcc-eval
