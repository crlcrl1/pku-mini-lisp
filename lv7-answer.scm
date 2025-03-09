(define (remove-if-not pred lst)
  (cond ((null? lst) '())
    ((pred (car lst)) (cons (car lst) (remove-if-not pred (cdr lst))))
    (else (remove-if-not pred (cdr lst)))))

(define (quicksort arr)
  (if (or (null? arr) (null? (cdr arr)))
    arr
    (let ((pivot (car arr))
           (rest (cdr arr)))
      (let ((smaller (remove-if-not (lambda (x) (< x pivot)) rest))
             (greater (remove-if-not (lambda (x) (>= x pivot)) rest)))
        (append
          (quicksort smaller)
          (list pivot)
          (quicksort greater))))))


(define sample '(12 71 2 15 29 82 87 8 18 66 81 25 63 97 40 3 93 58 53 31 47))

(print (quicksort sample))