for i in {1..6}; do
    ./pass1 tests/test$i/code tests/test$i/intermediate1 -o tests/test$i/opcode -s tests/test$i/symtab -O tests/test$i/objectCode
done
