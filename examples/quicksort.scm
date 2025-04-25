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