#!/bin/bash



# Specifies the clang-tidy executable

clang_tidy_executable="clang-tidy-19"



# Check for the verbosity and single file flags

verbose=false

single_file=false



display_results() {

    local tmp_file="$1"

    echo -e "\n\n"

    echo -e "- Use after free:            \t$(($(grep -c "Use of memory after it is freed" "$tmp_file") / 2))"

    echo -e "- Double frees:              \t$(($(grep -c "free released memory" "$tmp_file") / 2))"

    echo -e "- Null pointer dereferences: \t$(($(grep -c "dereference of a null pointer" "$tmp_file") / 2))"

    echo -e "- Undefined or garbage value:\t$(grep -c "garbage value" "$tmp_file")"

    echo -e "- Magic number:              \t$(grep -c "magic number" "$tmp_file")"

    echo -e "- Leak of memory:            \t$(grep -c "leak of memory" "$tmp_file")"

    echo -e "- Global variable:           \t$(grep -c globally "$tmp_file")"

    echo -e "- Narrowing conversion:      \t$(grep -c "narrowing conversion" "$tmp_file")"

    echo -e "- Implicit conversion:       \t$(grep -c "implicitly converted" "$tmp_file")"

    echo -e "- Param name too short:      \t$(grep -c "is too short" "$tmp_file")"

    echo -e "- Unused Returns:            \t$(grep -c "value returned by this function" "$tmp_file")"

    echo -e "- Differing parameter names: \t$(grep -c "differing parameters" "$tmp_file")"

    echo -e "\n\n"

}



# Parse through arguments

while getopts ":vf" opt; do

    case $opt in

    v) verbose=true ;;

    f) single_file=true ;;

    \?)

        echo "Invalid option: -$OPTARG" >&2

        exit 1

        ;;

    esac

done



# Shift the positional parameters to skip past the processed options

shift $((OPTIND - 1))



# Check if a directory argument is provided

if [ "$single_file" = false ] && [ -z "$1" ]; then

    echo "Usage: $0 [-v] [-s <file>] <directory>"

    echo "-v: Enable verbose mode"

    echo "-f: Specify single file"

    exit 1

fi



# Run clang-tidy for a single file or all files in the directory

directory="$1"

tmp_file=$(mktemp)



if [ "$single_file" = true ]; then

    # Process a single file

    if [ -z "$directory" ]; then

        echo "Usage: $0 [-v] [-f] <file>"

        exit 1

    fi

    if [ "$verbose" = true ]; then

        $clang_tidy_executable --checks="cert*, bugprone*, misc*, readability*" "$directory" -- -std=gnu99

    else

        $clang_tidy_executable --checks="cert*, bugprone*, misc*, readability*" "$directory" -- -std=gnu99 >"$tmp_file"

        display_results "$tmp_file"

    fi

else

    # Process all files in the directory

    if [ "$verbose" = true ]; then

        find "$directory" -type f -name '*.c' -exec $clang_tidy_executable --checks="cert*, bugprone*, misc*, readability*" {} +

    else

        find "$directory" -type f -name '*.c' -exec $clang_tidy_executable --checks="cert*, bugprone*, misc*, readability*" {} + >"$tmp_file"

        display_results "$tmp_file"

    fi

fi



# Remove the temporary file (if not in verbose mode)

[ "$verbose" = false ] && rm "$tmp_file"