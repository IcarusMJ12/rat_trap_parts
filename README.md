Gameplay
========
You start with one "current" 3-letter word, random or manually specified.
Each step, you create successive new words by choosing any current word, adding
any one letter of your choice to it, and then making an anagram to form a new
word.  e.g. 'rat' can be made into 'trap'; 'trap' can be made into 'parts'. This
new word goes into the "current" area and can be used in the next step.  The
used up word goes into the "prior" area and is only used for scoring.
As part of a step you may also split the word into two or more, as long as they
are valid and you still add a letter and make an anagram.
Words may never be repeated, and are tracked for uniqueness using their base
form. e.g. 'rat' cannot be changed into 'rats' because they are the same word.
To take a step, type '<current word> <new words separated by spaces><Enter>'
e.g. 'parts tar sip'

Valid Words
===========
Words must be at least 3 characters long, and must consist of lowercase letters
a-z.  This precludes proper names.

Scoring
=======
At the end of the game, each word in the "prior" section is worth points equal
to its length minus 3.  Words in the "current" section are worth double.  Thus,
it may not be worthwhile splitting a long word before terminating the game.
