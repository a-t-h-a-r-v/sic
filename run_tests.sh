make clean
make pass1
make pass2
for i in {1..6}; do
    find tests/test$i -type f ! -name 'code' ! -name 'opcode' -exec rm -v {} \;
    ./pass1 tests/test$i/code tests/test$i/intermediate1 -o tests/test$i/opcode -s tests/test$i/symtab -O tests/test$i/objectCode
    ./pass2 tests/test$i/intermediate1 tests/test$i/opcode tests/test$i/symtab -i tests/test$i/intermediate2 -O tests/test$i/objectCode
done
