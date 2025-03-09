#!/usr/bin/env bats
current_user=$(id -u)

start_server() {
  if pgrep -u "$current_user" "dsh" > /dev/null; then
    pkill -u "$current_user" "dsh"
  fi

  ./dsh -s &
}

stop_server() {
  # Stop the server
  if pgrep -u "$current_user" "dsh" > /dev/null; then
    pkill -u "$current_user" "dsh"
  fi
}

setup(){
    start_server
}

teardown(){
    stop_server
}

@test "Client can connect to server" {
    run ./dsh -c <<EOF              
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can send a command to the server" {
    res=$(ls)
    run ./dsh -c <<EOF     
ls         
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    res=$(echo "$res" | tr -d '[:space:]')
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>${res}dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can send a piped command to the server" {
    res=$(echo "hi" | wc)
    run ./dsh -c <<EOF     
echo "hi" | wc        
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    res=$(echo "$res" | tr -d '[:space:]')
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>${res}dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can send a command with arguments to the server" {
    res=$(echo "hi")
    run ./dsh -c <<EOF     
echo "hi"    
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    res=$(echo "$res" | tr -d '[:space:]')
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>${res}dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can send a piped command with arguments to the server" {
    res=$(echo "hi" | wc -l)
    run ./dsh -c <<EOF     
echo "hi" | wc -l       
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    res=$(echo "$res" | tr -d '[:space:]')
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>${res}dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can stop the server" {
    run ./dsh -c <<EOF        
echo "stop-server"
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>stop-serverdsh4>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can change directory" {
    p=$(pwd)
    d=$(ls ..)
    cd $p
    run ./dsh -c <<EOF        
cd ..
ls
exit
EOF

    d=$(echo "$d" | tr -d '[:space:]')
    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>dsh4>${d}dsh4>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client < Redirection works" {
    res=$(cat < dshlib.c | wc -l)
    run ./dsh -c <<EOF
cat < dshlib.c | wc -l
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>${res}dsh4>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client > Redirection works" {
    run ./dsh -c <<EOF
echo "bro code" > test
cat test
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    cat_test="bro code"
    cat_test=$(echo "$cat_test" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>dsh4>${cat_test}dsh4>cmdloopreturned0"
    rm -rf test
    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Client can connect to a specific port" {
    stop_server
    ./dsh -s -p 5050 &
    server_pid=$!
    run ./dsh -c -p 5050 <<EOF
exit
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output
    expected_output="socketclientmode:addr:127.0.0.1:5050dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Assert the expected output and status
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    kill "$server_pid"
}

@test "Client can connect to a specific ip" {
    stop_server
    ip=$(hostname -I)
    ./dsh -s -i $ip &
    server_pid=$!
    run ./dsh -c -i $ip << EOF
exit
EOF
    ip=$(echo "$ip" | tr -d '[:space:]')
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output
    expected_output="socketclientmode:addr:$ip:4545dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Assert the expected output and status
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    kill $server_pid
}

@test "Multiple clients can connect to server" {
    # First client connection
    run ./dsh -c << EOF
    exit
EOF
    out_one=$output

    run ./dsh -c << EOF
    exit
EOF
    out_two=$output
    out_one=$(echo "$out_one" | tr -d '[:space:]')
    out_two=$(echo "$out_two" | tr -d '[:space:]')

    echo "${out_one} -> ${out_two}"

    [ "$out_one" == "$out_two" ]
}

@test "Client can stop server" {
    run ./dsh -c << EOF
stop-server
exit
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output
    expected_output="socketclientmode:addr:127.0.0.1:4545dsh4>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Assert the expected output and status
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]

}





