#!/usr/bin/bash

#Taking mandatory arguments

submission_foler=$1
target_folder=$2
test_folder=$3
answer_folder=$4

# #Checking optional flags

verbose=false
no_execute=false
no_line_count=false
no_comment_count=false
no_function_count=false

shift 4

for arg in "$@"; do
    case $arg in
        -v)
            verbose=true
            ;;
        -noexecute)
            no_execute=true
            ;;
        -nolc)
            no_line_count=true
            ;;
        -nocc)
            no_comment_count=true
            ;;
        -nofc)
            no_function_count=true
            ;;
        -h)
            echo "Usage: script.sh <submission_folder> <target_folder> <test_folder> <answer_folder> [-v] [-noexecute] [-nolc] [-nocc] [-nofc]"
            echo "Options:"
            echo "  -v           Enable verbose mode"
            echo "  -noexecute   Do not execute the code"
            echo "  -nolc        Do not calculate lines of code"
            echo "  -nocc        Do not count comments"
            echo "  -nofc        Do not count functions"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            ;;
    esac
done



##Task A:Organize
answer_count=0
for i in $(ls $answer_folder); do
    answer_count=$((answer_count + 1))
done
cd $submission_foler
number_of_files=$(ls  -1 | wc -l)



# # unzip all the files

for i in *; do #as names have spaces, using ls file guloke split kore feltese. Tai we have to use a glob = *
  extension="${i##*.}"
    if [[ $extension = "zip" ]]; then
        unzip "$i" > /dev/null
        rm "$i"
    fi
  
done


mkdir -p ../$target_folder
mkdir -p ../$target_folder/C
mkdir -p ../$target_folder/C++
mkdir -p ../$target_folder/Java
mkdir -p ../$target_folder/Python
touch ../$target_folder/result.csv
echo "student_id,student_name,language,matched,not_matched,line_count,comment_count,function_count" > ../$target_folder/result.csv

find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.py" -o -name "*.java" \) | while IFS= read -r file; do
    # Get the file extension
    if [[ $file == "C" || $file == "CPP" || $file == "Java" || $file == "Python" ]]; then
        continue
    fi

    name="${file%%_*}"
    name="${name##*/}"
    roll_number="${file##*submission_}"
    roll_number="${roll_number%%/*}"
    extension="${file##*.}"
    line_count=$(wc -l < "$file")
    comment_count=0
    match=0
    unmatched=0
    file_name=$(basename "$file")
    file_name="${file_name%.*}"
    if [[ $verbose == true ]]; then
        echo "Organizing files of $roll_number"
    fi
    if [[ $extension == "c" || $extension == "cpp" || $extension == "java" ]]; then
        comment_count=$(grep -c "//" "$file")
    elif [[ $extension == "py" ]]; then
        comment_count=$(grep -c "#" "$file")
    fi


    # Create the target directory based on the file extension
    if [[ $extension == "c" ]]; then
        mkdir -p ../$target_folder/C/"$roll_number"
        cp "$file" ../$target_folder/C/"$roll_number"/"main.c"
    elif [[ $extension == "cpp" ]]; then
        mkdir -p ../$target_folder/C++/"$roll_number"
        cp "$file" ../$target_folder/C++/"$roll_number"/"main.cpp"
    elif [[ $extension == "java" ]]; then
        mkdir -p ../$target_folder/Java/"$roll_number"
        cp "$file" ../$target_folder/Java/"$roll_number"/"Main.java"
    elif [[ $extension == "py" ]]; then
        mkdir -p ../$target_folder/Python/"$roll_number"
        cp "$file" ../$target_folder/Python/"$roll_number"/"main.py"
    else
        echo "Unknown file type: $file"
    fi

    ## Execute the code if the flag is set
    if [[ $no_execute == false ]]; then
        count=1
        echo "Executing code of $roll_number"
        if [[ $extension == "c" ]]; then
        cd "../$target_folder/C/$roll_number"
        gcc main.c -o main.out
        for i in $(ls ../../../$test_folder); do
            ./main.out < ../../../$test_folder/$i > "out$count.txt"
            if diff -q "out$count.txt" "../../../$answer_folder/ans$count.txt" > /dev/null; then
                match=$((match + 1))
            else
                unmatched=$((unmatched + 1))
            fi
            count=$((count + 1))
        done
        fi
        if [[ $extension == "cpp" ]]; then
            cd "../$target_folder/C++/$roll_number"
            g++ main.cpp -o main.out
            for i in $(ls ../../../$test_folder); do
                ./main.out < ../../../$test_folder/$i > "out$count.txt"
                if diff -q "out$count.txt" "../../../$answer_folder/ans$count.txt">/dev/null; then
                    match=$((match + 1))
                else
                    unmatched=$((unmatched + 1))
                fi

                count=$((count + 1))
            done
        fi
        if [[ $extension == "java" ]]; then
            cd "../$target_folder/Java/$roll_number"
            javac Main.java 
            for i in $(ls ../../../$test_folder); do
                java Main < ../../../$test_folder/$i > "out$count.txt"
                if diff -q "out$count.txt" "../../../$answer_folder/ans$count.txt">/dev/null; then
                    match=$((match + 1))
                else
                    unmatched=$((unmatched + 1))
                fi
                count=$((count + 1))
            done
        fi
        if [[ $extension == "py" ]]; then
            cd "../$target_folder/Python/$roll_number"
            for i in $(ls ../../../$test_folder); do
                python3 main.py < ../../../$test_folder/$i > "out$count.txt"
                if diff -q "out$count.txt" "../../../$answer_folder/ans$count.txt">/dev/null; then
                    match=$((match + 1))
                else
                    unmatched=$((unmatched + 1))
                fi
                count=$((count + 1))
            done
        fi
        cd ../../../$submission_foler
    fi
    echo -n "$roll_number,$name" >> ../$target_folder/result.csv
    if [[ $extension == "c" ]]; then
        echo -n ",C" >> ../$target_folder/result.csv
    elif [[ $extension == "cpp" ]]; then
        echo -n ",C++" >> ../$target_folder/result.csv
    elif [[ $extension == "java" ]]; then
        echo -n ",Java" >> ../$target_folder/result.csv
    elif [[ $extension == "py" ]]; then
        echo -n ",Python" >> ../$target_folder/result.csv
    fi

    if [[ $no_execute == false ]]; then
        echo -n ",$match,$unmatched" >> ../$target_folder/result.csv
    else
        echo -n ",not calculated,not calculated" >> ../$target_folder/result.csv
    fi

    if [[ $no_line_count == false ]]; then
        echo -n ",$line_count" >> ../$target_folder/result.csv
    else
        echo -n "not calculated" >> ../$target_folder/result.csv
    fi

    if [[ $no_comment_count == false ]]; then
        echo -n ",$comment_count" >> ../$target_folder/result.csv
    else
        echo -n "not calculated" >> ../$target_folder/result.csv
    fi

    if [[ $no_function_count == false ]]; then
        function_count=$(grep -c "def " "$file")
        echo -n ",$function_count" >> ../$target_folder/result.csv
    else
        echo -n "not calculated" >> ../$target_folder/result.csv
    fi

    echo "" >> ../$target_folder/result.csv 
done

echo "All submissions processed successfully"




