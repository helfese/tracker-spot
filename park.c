char *del_blank(char *str) {
    char *ptr = str;
    int prev_blank, i, j, null;
    prev_blank = i = j = null = BASE;

    // Reset comma count
    comma_count = BASE;

    // Iterate through the string
    while (*ptr) {
        // Count commas
        if (*ptr == STR_CDEF)
            comma_count++;

        // Process characters except within quoted parts
        if (comma_count != FRST) {
            // Remove leading blanks
            if (isspace(*ptr))
                if (!prev_blank)
                    str[prev_blank = i++] = BLNK_CHR;

            // Copy non-blank characters
            if (!isspace(*ptr) && (prev_blank = null) == BASE)
                str[i++] = *ptr;
        } else {
            // Copy characters within quoted parts
            str[i++] = *ptr;
        }
        ptr++;
    }

    // Terminate the string
    str[i] = STR_END;

    // Check if the resulting string is empty
    if (str[j] == BLNK_CHR && str[j + FRST] == STR_END)
        return NULL_STR;
    else if (str[j++] == BLNK_CHR && str[j++] == BLNK_CHR && str[j] == STR_END)
        return NULL_STR;

    // Return the modified string
    return str;
}
