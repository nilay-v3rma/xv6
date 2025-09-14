This branch contains the base xv6 codebase with a feature to press tab to autocomplete user level commands.

### Implementation details

- **Trie-based Autocomplete:**  
  The kernel maintains a trie data structure to efficiently support prefix-based command autocompletion. All available user commands are inserted into the trie during system initialization.

- **Command Discovery:**  
  At boot, the kernel scans the root directory `/` (of the xv6 file structure not the host) for user-level commands (similar to the output of the `ls` command). Each command found is added to the trie for fast lookup.

- **Tab Key Handling:**  
  When the user presses the Tab key at the shell prompt, the kernel intercepts the input and uses the trie to search for possible command completions based on the current input prefix.

- **Unique Match Completion:**  
  If there is a unique match for the prefix, the kernel automatically completes the command in the input buffer.

- **Multiple Matches:**  
  If multiple matches are found, the kernel displays all possible completions to the user.

- **Integration:**  
  The autocomplete logic is implemented in the kernel’s `console.c` file. The trie is initialized and populated during system startup, after the file system is ready.

- **Limitations:**  
  - Only commands present in the xv6 file system at boot are available for autocompletion.
  - The feature currently supports autocompletion for the first word (the command) only.

### Usage

- Start xv6 as usual -> `make clean qemu`.
- At the shell prompt, begin typing a command and press the Tab key to autocomplete.
- If a unique command matches the prefix, it will be completed automatically.
- If no match is found, nothing happens or a bell may sound.

---

That's all! <br>
Feel free to share any thoughts or feedbacks! :)