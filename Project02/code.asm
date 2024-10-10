        lwcl $ft0,0($0) //0.9
        addi $t0,$0,64 //n
        addi $ti,$0,0 //i
        lwcl $ft1,4($0) //0.5
        addi $s0,$0,8 //数组指针
L1:
        beq $t1,$t0,done
        lwcl $fs0,0($s0)
        mul.s $fs0,$ft0,$fs0
        add.s $fs0,$ft1,$fs0
        swcl $fs0,0($s0)
        addi $s0,$s0,4
        addi $t1,$t1,1
        j L1
done:
        jr $ra

