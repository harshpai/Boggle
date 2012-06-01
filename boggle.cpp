/* File: boggle.cpp
 * ----------------
 * Harsh Pai
 */
 
#include "genlib.h"
#include "simpio.h"
#include <iostream>
#include "extgraph.h"
#include "strutils.h"
#include "grid.h"
#include "set.h"
#include "gboggle.h"
#include "random.h"
#include "lexicon.h"
#include "sound.h"

string StandardCubes[16]  = 
{"AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS", "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
 "DISTTY", "EEGHNW", "EEINSU", "EHRTVW", "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"};
 
string BigBoggleCubes[25]  = 
{"AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM", "AEEGMU", "AEGMNN", "AFIRSY", 
"BJKQXZ", "CCNSTW", "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT", "DHHLOR", 
"DHLNOR", "EIIITT", "EMOTTT", "ENSSSU", "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"};

//strucuture that holds 
//the coordinates of a cube 
//in board grid
struct cubeT{
	int row,col;
};

//pause to highlight cubes
double const DELAY = 0.5;
//sound file names
string const DICE_RATTLE = "dice rattle";
string const TWEETLE = "tweetle";
string const NOT_FOOLING = "not fooling anyone";
string const DENIED = "denied";
string const WHOOPS = "whoops";
string const THATS_PATHETIC ="thats pathetic";
string const TINKERBELL ="tinkerbell";
string const YEAH_RIGHT = "yeah right";

//Sounds: dont know where to use them?
string const COME_ON_FASTER = "come on faster";
string const EXCELLENT ="excellent";
string const IDIOT ="idiot";
string const MOO="moo";
string const NOT ="not";
string const OH_REALLY ="oh really";
string const YAH_AS_IF ="yah as if";

//function protoypes
static void Welcome();
void GiveInstructions();
bool GetYesOrNo(string prompt);
bool SetSound();
int GetBoggleSize();
Grid<char> SetupGameBoard(int dimension);
bool ForceBoardConfig();
void ManuallySetupBoard(Grid<char> &board);
string GetBoardString(int boardSize);
void RadndomSetupBoard(Grid<char> &board);
void ShuffleCubes(Grid<char> &board);
void SwapCubes(Grid<char> &board, int row, int col, int randomRow, int randomCol);
char ChooseRandomCubeFace(string cube);
void SetCubeFaces(Grid<char> &board, string cubes[]);
Set<string> PlayHumanTurn(Grid<char> &board,Lexicon &lexicon);
bool CheckWord(string word,Lexicon &lexicon,Set<string> &guessedWords,Grid<char> &board);
int CubeCmpFn(cubeT one, cubeT two);
bool RecursiveMakeWord(string word, int index, Grid<char> &board,cubeT curCube, Set<cubeT> &usedCubes,Vector<cubeT> &wordPath);
bool CanMakeWord(string word,Grid<char> &board,Set<cubeT> &usedCubes,Vector<cubeT> &wordPath);
Set<cubeT> GetAdjoiningCubes(cubeT cube,Set<cubeT> &usedCubes,Grid<char> &board);
bool IsInBounds(cubeT cube,Grid<char> &board);
void HighlightWord(Vector<cubeT> &wordPath);
void PlayComputerTurn(Grid<char> &board,Lexicon &lexicon,Set<string> guessedWords);
void RecursiveSearchWords(string word, cubeT curCube, Set<cubeT> &usedCubes, Lexicon &lexicon, Set<string> &guessedWords, Grid<char> &board );
bool CheckQu(string word, int &index);
string AddLetter(string word,char nextLetter);
void PlayBoggle(int dimension);

int main()
{

	Randomize();
	SetWindowSize(9, 5);
	InitGraphics();
	Welcome();
	SetSoundOn(SetSound());
	GiveInstructions();
	PlayBoggle(GetBoggleSize());
	return 0;
}

void PlayBoggle(int dimension){
	string prompt = "Would you like to play again? ";
	Lexicon lexicon("lexicon.dat");
	while(true){
		Grid<char> board = SetupGameBoard(dimension);
		Set<string> guessedWords = PlayHumanTurn(board,lexicon);
		PlayComputerTurn(board,lexicon,guessedWords);		
		if(!GetYesOrNo(prompt))		return;
		InitGraphics();
	}
}

//Function:	PlayComputerTurn
//Usage:	PlayComputerTurn(board,lexicon,guessedWords);
//------------------------------------------------------
//This function plays the compuetrs turn and prints all the missed 
//words on the board. The implementation at this level is a wrapper
//function that calls the recursive bactracker with appropriate 
//arguments.
void PlayComputerTurn(Grid<char> &board,Lexicon &lexicon,Set<string> guessedWords){

	PlayNamedSound(TWEETLE);

	cubeT firstCube;
	Set<cubeT> usedCubes(CubeCmpFn);	
	for (int row = 0; row < board.numRows(); row++){
		for (int col = 0; col < board.numCols(); col++){				
			firstCube.row = row;
			firstCube.col = col;
			usedCubes.clear();
			usedCubes.add(firstCube);
			string firstLetter = AddLetter("",board(row,col));
			RecursiveSearchWords(firstLetter,firstCube,usedCubes,lexicon,guessedWords,board);
		}
	}
}

//Function: RecursiveSearchWords
//Usage:	RecursiveSearchWords(firstLetter,firstCube,usedCubes,lexicon,guessedWords,board);
//----------------------------------------------------------------------------
//This is the recursive backtracking function that does the real work for PlayComputerTurn.
//The function uses containsPrefix in the Lexicon class to stop going into dead ends.
//It prints a word if it satisifes the games rules viz. length > 3, not repeated, valid word
//It then appends adjoining letter and continues to search recursively.
void RecursiveSearchWords(string word, cubeT curCube, Set<cubeT> &usedCubes, Lexicon &lexicon, Set<string> &guessedWords, Grid<char> &board ){

	if(!lexicon.containsPrefix(word))		return;

	if(word.length()>3 && lexicon.containsWord(word) && !guessedWords.contains(word)){		
		guessedWords.add(word);
		RecordWordForPlayer(word,Computer);		
	}

	Set<cubeT> adjCubes = GetAdjoiningCubes(curCube,usedCubes,board);
	
	foreach(cubeT cube in adjCubes){	
		usedCubes.add(cube);

		//Extension: Make the Q a useful letter
		string next = AddLetter(word,board(cube.row,cube.col));

		RecursiveSearchWords(next, cube, usedCubes, lexicon, guessedWords, board );
		usedCubes.remove(cube);
	}
}

//Extension: Make the Q a useful letter
//Function: AddLetter
//Usage:	string next = AddLetter(word,board(cube.row,cube.col));
//----------------------------------------------------------------
//Returns a string that has the nextLetter appended to the word string.
//If the nextLetter is Q then appends QU instaed of just Q.
string AddLetter(string word,char nextLetter){

	if(nextLetter=='Q')		return word+"QU";
	return word+nextLetter;
}

//Function:	PlayHumanTurn
//Usage:	PlayHumanTurn(board,lexicon,guessedWords);
//--------------------------------------------------
//This is the function that allows user to play a turn. It takes
//multiple word inputs from the user and verifies if the word is valid 
//according to the rules of boggle. For each valid word, user's 
//word list and score are updated. For an invalid word an appropriate 
//message is shown to the user. The function returns a set of guessed 
//words when the player enters a lone carriage return.
Set<string> PlayHumanTurn(Grid<char> &board,Lexicon &lexicon){
	cout<<"\nOk, take all the time you want and find all the words you can!"<<endl
		<<"Signal that you're finished by entering an empty line."<<endl<<endl;
		
	Set<string> guessedWords;

	while(true){
		cout<<"Enter a word: ";
		string word = ConvertToUpperCase(GetLine());
		if(word.empty())			return guessedWords;
		if(CheckWord(word, lexicon, guessedWords,board)){
			PlayNamedSound(TINKERBELL);
			guessedWords.add(word);
			RecordWordForPlayer(word,Human);
		}			
	}
}

//Function: CanMakeWord
//Usage:	if(!CanMakeWord(word,board,usedCubes,wordPath))...
//-------------------------------------------------------
//This predicate function uses recursive backtracking to find
//if the given argument word can be formed on the board or not.
//At this level the implementation is a wrapper function that 
//calls the recursive function with appropriate arguments 
//for every cube on the board.
bool CanMakeWord(string word,Grid<char> &board,Set<cubeT> &usedCubes,Vector<cubeT> &wordPath){

	cubeT curCube;
	
	for (int row = 0; row < board.numRows(); row++){
		for (int col = 0; col < board.numCols(); col++){			
			curCube.row = row;
			curCube.col = col;
			usedCubes.clear();
			if(RecursiveMakeWord(word,0,board,curCube,usedCubes,wordPath))		return true;
		}
	}
	return false;
}

//Function:	HighlightWord
//Usage:	HighlightWord(usedCubes);
//--------------------------------------------------
//This function takes in a set of cubes as an argument and 
//highlights them on the baord for DELAY seconds. 
//It also draws lines between words indicating its path.
void HighlightWord(Vector<cubeT> &wordPath){

	cubeT prev,cur;

	for (int i = wordPath.size()-1 ; i >=0 ; i--){
		HighlightCube(wordPath[i].row,wordPath[i].col,true);
		
		if(i == wordPath.size()-1) prev = wordPath[i];
		else{
			cur = wordPath[i];
			DrawLineConnectingCubes(prev.row,prev.col,cur.row,cur.col);
			prev = cur;
		}
	}
	
	Pause(DELAY);

	for (int i = 0 ; i < wordPath.size(); i++)
		HighlightCube(wordPath[i].row,wordPath[i].col,false);
}

//Function: RecursiveMakeWord
//Usage:	if(RecursiveMakeWord(word,0,board,curCube,usedCubes,wordPath))...
//----------------------------------------------------------------
//This is the recursive function that does all the real work for CanMakeWord.
//It uses recursive backtracking to quickly prune bad choices. The algorithm 
//can be described as follows:
//	If all letters have been found in the given word return true
//	If the letter at the current index is not the same as on the cube return false
//	Else	For(all valid adjoining cubes)
//				Add the current cube to the usedCube list
//				Recurse on the adjoining cube for the next letter 
//				If the recursion worked out return true 
//				Else remove the adjoining cube from the usedCube list
//	If no adjoining cubes worked return false.			
bool RecursiveMakeWord(string word, int index, Grid<char> &board,cubeT curCube, Set<cubeT> &usedCubes,Vector<cubeT> &wordPath){

	if(index == word.size()-1)			
	{
		if(word[index] == board(curCube.row,curCube.col))
		{
			wordPath.add(curCube);
			return true;
		}		
		return false;
	}

	if(word[index] != board(curCube.row,curCube.col))		return false;

	//Extension: Make the Q a useful letter
	if(!CheckQu(word,index))	return false;

	//adjoining cube
	Set<cubeT> adjCubes = GetAdjoiningCubes(curCube,usedCubes,board);
	Set<cubeT>::Iterator it = adjCubes.iterator();

	while(it.hasNext()){
		cubeT neighbour = it.next();
		usedCubes.add(curCube);
		if(RecursiveMakeWord(word,index+1,board,neighbour,usedCubes,wordPath)){
			wordPath.add(curCube);		
			return true;
		}
		usedCubes.remove(curCube);
	}

	return false;
}

//Extension Make the Q a useful letter
//Function:	CheckQu
//Usage:	if(!CheckQu(word,index))...
//------------------------------------------------
//This predicate function false if the current letter is Q 
//and it is the last letter or it is not followed by U.
//If the current letter is Q and is followed by U
//then it increments the index and return true.
bool CheckQu(string word, int &index){
	
	if(word[index] != 'Q')	return true;

	if(index == word.length()-1 || word[index+1] != 'U')	return false;

	index++;
	return true;
}

//Function: GetAdjoiningCubes
//Usage: Set<cubeT> adjCubes = GetAdjoiningCubes(curCube,usedCubes,board);
//--------------------------------------------------------------------------
//This function takes in a cube, a set of used cubes and the board as argument.
//It returns a set of adjoining unused cubes on the board. Two cubes adjoin 
//if they are next to each other horizontally, vertically, or diagonally. 
Set<cubeT> GetAdjoiningCubes(cubeT cube,Set<cubeT> &usedCubes,Grid<char> &board){
	Set<cubeT> adjoiningCubes(CubeCmpFn);

	for (int i = -1; i <= 1 ; i++){
		for (int j = -1; j <= 1; j++){
			cubeT adjoiningCube = cube;
			adjoiningCube.row+=i;
			adjoiningCube.col+=j;
			if(!(i==0 && j==0) && IsInBounds(adjoiningCube,board) && !usedCubes.contains(adjoiningCube))
				adjoiningCubes.add(adjoiningCube);
		}
	}

	return adjoiningCubes;
}

//Function: IsInBounds
//Usage:	if(!(i==0 && j==0) && IsInBounds(adjoiningCube,board) ...
//------------------------------------------------------------------------
//Predicate function returns true if the given cube argument lies within 
//the bounds of the board argument
bool IsInBounds(cubeT cube,Grid<char> &board){

	return (cube.row>=0 && cube.col >=0 && cube.row < board.numRows() && cube.col < board.numCols());
}

//Function: CheckWord
//Usage:	if(CheckWord(word, lexicon, guessedWords, board))...
//-------------------------------------------------
//This predicate function returns true if the given word satisfies all of the 
//following conditions:
//• it is at least four letters long
//• it is contained in the English lexicon
//• it has not already been included in the player’s word list (even if there is an alternate path
//on the board to form the same word, the word is counted at most once)
//• it can be formed on the board (i.e., it is composed of adjoining letters and each cube is used
//at most once)
//If any of these conditions fail, the word is rejected anf the function returns false.
bool CheckWord(string word,Lexicon &lexicon,Set<string> &guessedWords,Grid<char> &board){
	
	Set<cubeT> usedCubes(CubeCmpFn);
	Vector<cubeT> wordPath;

	if(word.length()<4){
		PlayNamedSound(WHOOPS);
		cout<<"I'm sorry, but we have our standards"<<endl
			<<"That word does'nt meet the minimum word length."<<endl;
		return false;
	}
	if(!CanMakeWord(word,board,usedCubes,wordPath)){
		PlayNamedSound(DENIED);
		cout<<"You can't make that word!"<<endl;
		return false;
	}
	if(!lexicon.containsWord(word)){
		PlayNamedSound(NOT_FOOLING);
		cout<<"That's not a word!"<<endl;
		return false;
	}
	if(guessedWords.contains(word)){
		PlayNamedSound(THATS_PATHETIC);
		cout<<"You've already guessed that!"<<endl;
		return false;
	}

	HighlightWord(wordPath);
	return true;
}

//Function:	SetupGameBoard
//Usage:	Grid<char> board = SetupGameBoard();
//-----------------------------------------------------
//This function gives the user appropriate prompts and sets 
//up the game board either manually or random. It returns a 
//grid that contains the state of the initialized board.
Grid<char> SetupGameBoard(int dimension){

	Grid<char> board(dimension,dimension);						
	DrawBoard(dimension,dimension);

	if(ForceBoardConfig())			ManuallySetupBoard(board);
	else							RadndomSetupBoard(board);

	return board;
}

//Function: RadndomSetupBoard
//Usage:	... RadndomSetupBoard(board);
//---------------------------------------------------------
//This function sets up the board in a random manner. At this level, the
//implementation is a wrapper function that calls  SetCubeFaces with 
//approprite arguments, which sets up all the cube faces. It then calls 
//ShuffleCubes, which shuffles the cubes on the board.
void RadndomSetupBoard(Grid<char> &board){

	PlayNamedSound(DICE_RATTLE);

	if(board.numRows()==MAX_DIMENSION)		SetCubeFaces(board, BigBoggleCubes);
	else									SetCubeFaces(board, StandardCubes);

	ShuffleCubes(board);
}

//Function: ShuffleCubes
//Usage:	ShuffleCubes(board);
//-----------------------------------------------------------
//This function shuffles the cubes in the argument grid. It iterates through 
//each grid element and swaps the current grid element with a random grid element.
void ShuffleCubes(Grid<char> &board){

	for (int row = 0; row < board.numRows(); row++){
		for (int col = 0; col < board.numCols(); col++){
			int randomRow = RandomInteger(row,board.numRows()-1);
			int randomCol = RandomInteger(col,board.numCols()-1);
			SwapCubes(board,row,col,randomRow,randomCol);
			LabelCube(row,col,board(row,col));
		}
	}
}

//Function:	SwapCubes
//Usage:	SwapCubes(board,row,col,randomRow,randomCol);
//----------------------------------------------------------
//This function is used in shuffling cubes on the board. It swaps two 
//cubes at the locations specified in the arguments.
void SwapCubes(Grid<char> &board, int row, int col, int randomRow, int randomCol){

	char temp = board(row,col);
	board(row,col) = board(randomRow,randomCol);
	board(randomRow,randomCol) = temp;
}

//Function: SetCubeFaces
//Usage:	if(board.numRows()==MAX_DIMENSION)		SetCubeFaces(board, BigBoggleCubes);
//-------------------------------------------------------------------------------------
//Modifies the argument board such that each cube has a random face. This function iterates
//through each board element and chooses a random character to display on each cube from the 
//given array of cube strings. The size of the cube string array must be the same as the size 
//of the board grid.
void SetCubeFaces(Grid<char> &board, string cubes[]){
	
	int index =0;
	for (int row = 0; row < board.numRows(); row++){
		for (int col = 0; col < board.numCols(); col++){
			board(row,col) = ChooseRandomCubeFace(cubes[index]);				
			index++;
		}
	}	
}

//Function: ChooseRandomCubeFace
//Usage:	board(row,col) = ChooseRandomCubeFace(cubes[index]);
//-----------------------------------------------------
//Returns a random face for a given cube.
char ChooseRandomCubeFace(string cube){
	return cube[RandomInteger(0,cube.length()-1)];
}

//Function: ManuallySetupBoard
//Usage:	if(ForceBoardConfig())			ManuallySetupBoard(board);
//--------------------------------------------------------
//When the user chooses to enter a custom board configuration, this function
//lets the user enter a string of characters, representing the cubes from left 
//to right, top to bottom. It does so by calling GetBoardString. It then updates 
//the board state in the grid and on the screen.
void ManuallySetupBoard(Grid<char> &board){
	
	string cubes = GetBoardString(board.numRows());
	int index=0;
	for (int row = 0; row < board.numRows(); row++){
		for (int col = 0; col < board.numCols(); col++){
			board(row,col) = cubes[index];
			LabelCube(row,col,cubes[index]);
			index++;
		}
	}
}


//Function:	GetBoardString
//Usage:	string cubes = GetBoardString(board.numRows());
//-----------------------------------------------------------
//This function takes the board size as an argument and returns a string 
//that will be used to fill the cubes on board. It prompts the user for input.
//It then verifies that this string is long enough to fill the board and re-prompts
//if it is too short. It does not verify if the entered characters are legal letters.
string GetBoardString(int boardSize){
	cout<<"\nEnter a "<<boardSize*boardSize<<"-character string to identify which letters you want on the cubes."
		<<"\nThe first "<<boardSize<<" letters are the cubes on the top row from left to right"
		<<"\nnext "<<boardSize<<" letters are the second row, etc."
		<<"\nEnter the string: ";
	string boardString;

	while(true){
		
		boardString = GetLine();

		if(boardString.length() >= boardSize*boardSize)		return ConvertToUpperCase(boardString);

		cout<<"String must include "<<boardSize*boardSize<<" charcters! Try again: ";
	}
}

//Function: ForceBoardConfig
//Usage:	if(ForceBoardConfig())	...
//-------------------------------------------------
//This predicate function asks user if board configuration will
//be manual.
bool ForceBoardConfig(){

	cout<<"\nI'll give you a chance to set up the board to your specification."<<endl
		<<"This makes it easier to confirm your boggle program is working."<<endl;
	string prompt="Do you want to force board configuration? ";
	return GetYesOrNo(prompt);
}

//Function: ChooseBoggleSize
//Usage:	int dimension = GetBoggleSize();
//-------------------------------------------------
//Prompts the user to select big or standard boggle. 
//Returns the boggle board size.
int GetBoggleSize(){

	string prompt = "\nYou can choose standard Boggle (on 4x4 grid) or Big Boggle (5x5).\nWould you like Big Boggle? ";	

	if(GetYesOrNo( prompt))	return MAX_DIMENSION;

	return MAX_DIMENSION-1;

}

//Function: SetSound
//Usage:	bool sound = SetSound()
//-----------------------------------------------
//This predicate function prompts user if sound effects should be used.
//Returns user response as boolean.
bool SetSound(){
	
	string prompt = "Would you like sound? ";
	return GetYesOrNo( prompt);
}

//Function: GiveInstructions
//Usage:	GiveInstructions();
//---------------------------------------------------
//Function asks if user needs instruction to play. 
//Displays instructions if user answers yes.
void GiveInstructions()
{
	string prompt ="Do you need instructions? ";
	
	if(!GetYesOrNo(prompt)){
		PlayNamedSound(YEAH_RIGHT);	
		return;
	}
    cout << endl << "The boggle board is a grid onto which I will randomly distribute " 
	 << "cubes. These 6-sided cubes have letters rather than numbers on the faces, " 
	 << "creating a grid of letters on which you try to form words. You go first, " 
	 << "entering all the words you can find that are formed by tracing adjoining " 
	 << "letters. Two letters adjoin if they are next to each other horizontally, " 
	 << "vertically, or diagonally. A letter can only be used once in the word. Words "
	 << "must be at least 4 letters long and can only be counted once. You score points "
	 << "based on word length: a 4-letter word is worth 1 point, 5-letters earn 2 "
	 << "points, and so on. After your puny brain is exhausted, I, the super computer, "
	 << "will find all the remaining words and double or triple your paltry score." << endl;
	
    cout << "\nHit return when you're ready...";
    GetLine();
}

//Function: Welcome
//Usage:	Welcome();
//-----------------------------------------------------------
//Displays welcome message to the user.
static void Welcome()
{
    cout << "Welcome!  You're about to play an intense game of mind-numbing Boggle. " 
	 << "The good news is that you might improve your vocabulary a bit.  The "
	 << "bad news is that you're probably going to lose miserably to this little "
	 << "dictionary-toting hunk of silicon.  If only YOU had a gig of RAM..." << endl << endl;
}


//Function: GetYesOrNo
//Usage:	if(!GetYesOrNo(prompt))	...
//----------------------------------------------------
//This predicate function asks a yes or no question specified 
//by the prompt. It returns true if the input begins with y or
//false if the answer begins with n. Re prompts in case of 
//invalid input.
bool GetYesOrNo(string prompt){

	string input;
	while(true){
		cout<<prompt;
		input = ConvertToLowerCase(GetLine());
		if(!input.empty())
			if(input[0] == 'y')			return true;
			if(input[0] == 'n')			return false;
		cout<<"Please answer yes or no."<<endl;
	}
}

//Function:		CmpPoints
//Usage:		Set<cubeT> usedCubes(CubeCmpFn);
//---------------------------------------------------------------
//This function specifies the comparision between two cubes and 
//is passed as an argument to the constructor when creating a set 
//of cubes. It is the client callback function defined for the 
//set of cubes.
int CubeCmpFn(cubeT one, cubeT two)
{
	if (one.row == two.row && one.col == two.col)
	{
		return 0;
	}
	else if (one.row < two.row)
	{
		return -1;
	}
	else if (one.row == two.row && one.col < two.col)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}
