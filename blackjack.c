/*
** GTK Must be installed
** I compile with: gcc blackjack.c `pkg-config --cflags --libs gtk+-3.0` -o blackjack
**
** Written by Jonas Blumer, Nov 2013 - Jan 2014
** On Ubuntu
*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>


int numPlayerAces; // how many aces do player
int numDealerAces; // and dealer have

int totalWinsPlayer; // how many times have you/dealer won
int totalWinsDealer;

int totalPointsPlayer; // total points
int totalPointsDealer;

int usedCards[52]; // array to hold a max of 52 cards that have already been drawn (overkill amount)
int cardCounter = 0; // how many cards have been drawn

GtkWidget *playerBox; // the "box" widgets where the cards reside
GtkWidget *dealerBox;

GtkWidget *totalPlayerLabel; // total points label
GtkWidget *totalDealerLabel;
GtkWidget *totalWinsPlayerLabel; // total wins label
GtkWidget *totalWinsDealerLabel;

GtkWidget *stayButton; // the stay button widget
GtkWidget *drawCardButton; // draw button widget

GtkWidget *stateLabel; // the game state -> win/lose in big red lettera

/*
** the game state that is checked every turn
*/
typedef enum {WIN, LOSE, TIE, IN_GAME} gameState;
gameState myGameState = IN_GAME;


char buf[3]; // the buffer used for printing the points and wins. always reused

void drawCard(GtkWidget *button, GtkWidget *box); // the draw card function prototype



/*
** reset function always called upon restart or first game start
** resets all varuable values that need to be reset / initialized
*/
void reset(){
	// when game starts, gamestate is
	myGameState = IN_GAME;

	totalPointsDealer = 0;
	totalPointsPlayer = 0;
	numPlayerAces = 0;
	numDealerAces = 0;
	cardCounter = 0;

	
	

/*
	sprintf(buf, "<span foreground=\"#3d3d3d\"> <big><b>Dealer Points: %d</b></big></span>", totalPointsDealer);
	gtk_label_set_markup (GTK_LABEL (totalDealerLabel), buf);	
*/

	
	/*
	** set values in array to unrealistic numbers (which will later represent drawn cards between 1 and 52)
	*/	
	int j;
	for(j = 0; j < 52; j++){
		usedCards[j] = 666;

	}

	// draw the first card for the dealer
	drawCard(NULL, dealerBox);

	//GENERATE RANDOM SEED
	srand(time(NULL)); 

	
}


/*
** removes all cards from the player and dealer boxes
*/
void destroy(GtkWidget *button){
	GList *children, *iter;

	children = gtk_container_get_children(GTK_CONTAINER(playerBox));
	for(iter = children; iter != NULL; iter = g_list_next(iter))
	 	gtk_widget_destroy(GTK_WIDGET(iter->data));
	g_list_free(children);

	children = gtk_container_get_children(GTK_CONTAINER(dealerBox));
	for(iter = children; iter != NULL; iter = g_list_next(iter))
	 	gtk_widget_destroy(GTK_WIDGET(iter->data));
	g_list_free(children);

	reset();
}


/*
** CHECKS POINTS OF CARD
** if card name is between 1 and 4, it's a 2 -> return 2...etc
*/

int checkCardPoints(int i){


	if(i <= 4){
		return 2;
	}else if(i <= 8){
		return 3;		
	}else if(i <= 12){
		return 4;		
	}else if(i <= 16){
		return 5;		
	}else if(i <= 20){
		return 6;		
	}else if(i <= 24){
		return 7;		
	}else if(i <= 28){
		return 8;		
	}else if(i <= 32){
		return 9;		
	}else if(i <= 48){
		return 10;		
	}else if(i <= 52){
		return 11;		
	}
		
}

/*
** update the points label and check if 
*/

void updatePoints(GtkWidget *button, GtkWidget *label){
	
	//if the label passed into the function is the player's label
	if(label == totalPlayerLabel){
		if(totalPointsPlayer > 21){ // if the player has over 21 and he would lose
			if(numPlayerAces > 0){ //check if he has aces. if he does:
				do{				
					totalPointsPlayer -= 10; //change the points an ace gives to 1 (11 - 10) 
					numPlayerAces -= 1; //change number of aces -1
				}while((numPlayerAces > 0) && (totalPointsPlayer > 21)); // do this while the players points are over 21 and he still has aces left			
		
			
			}else{
				//if he has no aces left but his points are still over 21, change gamestate to game over
				myGameState = LOSE;
		
			}
		} 
		// update the player's points label
		sprintf(buf, "<span foreground=\"#3d3d3d\"> <big><b>Player Points: %d</b></big></span>", totalPointsPlayer);
		gtk_label_set_markup (GTK_LABEL (label), buf);	
	}

	//if the label passed into the function is the dealer's label
	if(label == totalDealerLabel){
		if(totalPointsDealer > 21){ //check his ace situation and handle accordingly
			if(numDealerAces > 0){
				do{
					printf("dealerPoints: %d\n", totalPointsDealer);
					totalPointsDealer -= 10;			
					numDealerAces -= 1;
				}while((numDealerAces > 0) && (totalPointsDealer > 21));
				

			}		
		}
		//update dealer's points label
		sprintf(buf, "<span foreground=\"#3d3d3d\"> <big><b>Dealer Points: %d</b></big></span>", totalPointsDealer);
		gtk_label_set_markup (GTK_LABEL (label), buf);		

	}

}

//show hidden buttons when restart is clicked
void enableButton(GtkWidget *resetButton, GtkWidget *button){

	gtk_widget_set_visible (GTK_WIDGET (button), 1);

}


//if you lose, hide the "stay" and "draw" buttons
void checkAndDisable(GtkWidget *button, GtkWidget *thisStayButton){


	if(totalPointsPlayer > 21){
		gtk_widget_set_visible (GTK_WIDGET (button), 0);
		myGameState = LOSE;
		
		if(thisStayButton != NULL){	//check if stayButton is passed
			gtk_widget_set_visible (GTK_WIDGET (thisStayButton), 0);

		}
	}		
}

//when the stay button is clicked, hide it!
void disableStayButton(GtkWidget *button){
	gtk_widget_set_visible (GTK_WIDGET (button), 0);
}

/*
** DRAWS CARDS (both meanings of the word)
*/

void drawCard(GtkWidget *button, GtkWidget *box){
int randomNumber;

	
	GtkWidget *image;	// make an image ready to hold the card graphic
	GdkPixbuf *pixbuf;	
	GError *error = NULL; // error needs to be initialized for the pixbuf...no one knows why...

	// make random number between 1 and 52
	randomNumber = rand();
	randomNumber %= 52;
	randomNumber += 1;
	//randomNumber %= 3;
	//randomNumber += 1;


	printf("rand: %d\n", randomNumber);

	//CHECK FOR DUPLICATE CARD AND REDRAW IF NECESSARY
	int duplicate = 0; // this int is used as a boolean..yes, not good;)
	do{ // while duplicate == 1
		 // reset the variable to 0

		
		/*
		** cardCounter is used as a counter for amount of cards already drawn.
		** while j is smaller or equal to the amount of drawn cards,
		** compare the value of the usedCards array with the newly generated
		** random number
		*/
		int j; // var for "for" loop counter
		for(j = 0; j < cardCounter; j++){ 
			printf("count: %d\n",j);
			if(usedCards[j] == randomNumber){ // if the card already exists in the array
				printf("REDRAW %d\n",randomNumber);
				randomNumber = rand();	// generate new random number
				randomNumber %= 52;
				randomNumber += 1;
				//printf("TO %d\n",randomNumber);
				duplicate = 1; // change boolean to "true" to repeat the check for the newly drawn card
				break;

			}else{
				duplicate = 0;

			}
		}
	}while(duplicate == 1);

	usedCards[cardCounter] = randomNumber; //add the new card to the array of used cards
	cardCounter++; // increase the array counter
	if(randomNumber>=49){ //if the card drawn was an ace
		if(box == dealerBox){	 //if the dealer box was passed into the draw card function,	
			numDealerAces +=1;// add one to the dealer's aces
		}else{
			numPlayerAces += 1;	//else to the player's
		}
	}
	if(box == playerBox){ //card for player
		totalPointsPlayer += checkCardPoints(randomNumber); //players points don't need to be updated here because they are when clicking the drawCard button
	}else{	
		totalPointsDealer += checkCardPoints(randomNumber);
		updatePoints(NULL, totalDealerLabel); // update points when dealer draws one

	}


	
	char rand[14]; 	//create array for the name of the file consisting of the random number, the path and .png
			// so "images/RANDNUM.png
	sprintf(rand, "images/%d.png", randomNumber); // fill rand with that thing

	pixbuf = gdk_pixbuf_new_from_file(rand, &error); 			// create the pixbuf of the image
	pixbuf = gdk_pixbuf_scale_simple(pixbuf, 100, 150, GDK_INTERP_BILINEAR); // scale to right size
	image = gtk_image_new_from_pixbuf(pixbuf);				// pass the pixbuf to the image
	gtk_container_add(GTK_CONTAINER(box), image);				// add the image/widget to the container that was passed to the drawCard function
	gtk_widget_show(image);							// make it visible


	
	

	
}

/* 
** DEALER DRAWS CARDS -> "STAY" button clicked
** this implements the dealer logic -> should i draw another card? did the gamestate change after drawing one? etc.
*/
void dealersTurn(GtkWidget *button){

	gtk_widget_set_visible (GTK_WIDGET (drawCardButton), 0); //hide the drawCard button -> it's the dealer's turn!
	//gtk_widget_set_visible (GTK_WIDGET (button), 1); //not needed!?
		
	// draw cards until points equal 17 or more
	if(totalPointsDealer <= 16){
		do{
			drawCard(NULL, dealerBox);	
		}while(totalPointsDealer < 17);
	}

	//if dealer has equal or more than 17 points
	if(totalPointsDealer > 16){
		if(totalPointsDealer > 21){ 	// if dealer has more than 21 points
			myGameState = WIN;	// player wins!
		
		}else{				//if player didn't win
			if(totalPointsDealer > totalPointsPlayer){ 	// check if dealer has more points than player
									// then player loses
				myGameState = LOSE;			
			}else if (totalPointsDealer == totalPointsPlayer){ 	// if not, check if both have same amount of points

				myGameState = TIE;			// that makes it a tie
			}else{						// and if it's not a tie									

				myGameState = WIN;			// then the player wins
			}			
		}
	}
}

//every time when any button is clicked, do what the button does (draw card/draw dealer card), then check game state to see if you win, lose or it's a tie

void checkGameState(GtkWidget *button, GtkWidget *label){
	if(myGameState == WIN){	
		gtk_label_set_markup (GTK_LABEL (label), "<span foreground=\"red\" font_size=\"77888\"><b>YOU WIN</b></span>"); //big red letters telling you what the hell is going on
		totalWinsPlayer += 1;	
	}
	if(myGameState == LOSE){
		gtk_label_set_markup (GTK_LABEL (label), "<span foreground=\"red\" font_size=\"77888\"><b>YOU LOSE</b></span>");
		totalWinsDealer += 1;	
	}
	if(myGameState == TIE){
		gtk_label_set_markup (GTK_LABEL (label), "<span foreground=\"red\" font_size=\"77888\"><b>NICE TIE</b></span>"); 
	}
	if(myGameState == IN_GAME){
		gtk_label_set_text(GTK_LABEL(label), "");	
	}
	
	// edit the labels displaying the number of time you/dealer won
	sprintf(buf, "<span foreground=\"#3d3d3d\"> <big><b>Dealer Wins: %d</b></big></span>", totalWinsDealer); 
	gtk_label_set_markup (GTK_LABEL (totalWinsDealerLabel), buf);
	sprintf(buf, "<span foreground=\"#3d3d3d\"> <big><b>Player Wins: %d</b></big></span>", totalWinsPlayer);
	gtk_label_set_markup (GTK_LABEL (totalWinsPlayerLabel), buf);

}

int main(int argc, char *argv[]){

	totalWinsPlayer = 0; //when the game starts, set total wins to 0
	totalWinsDealer = 0;
	
	//MAIN GTK LOOP START
	gtk_init(&argc, &argv); //initialize gtk -> main gtk loop starts here
	GtkSettings *default_settings = gtk_settings_get_default(); //set the settings to default
	g_object_set(default_settings, "gtk-button-images", TRUE, NULL);  //enable images on buttons

	// CREATE MAIN WINDOW
	GtkWidget *window; 
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Jaque Noire");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	//CREATE MAIN FRAME IN WINDOW
	GtkWidget *frame;
	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);

	//BACKGROUND WOOD IMAGE
	GdkPixbuf *backgrndPixbuf;
	GtkWidget *backgrndImage;
	GError *error = NULL;
	backgrndPixbuf = gdk_pixbuf_new_from_file("images/wood.png", &error);
	backgrndPixbuf = gdk_pixbuf_scale_simple(backgrndPixbuf, 800, 500, GDK_INTERP_BILINEAR); 	
	backgrndImage = gtk_image_new_from_pixbuf(backgrndPixbuf);
	gtk_container_add(GTK_CONTAINER(frame), backgrndImage);

	//GREEN FELT BOX BACKGROUNDS
	GdkPixbuf *boxFeldPixbuf;
	GtkWidget *boxFeltImagePlayer;
	GtkWidget *boxFeltImageDealer;
	boxFeldPixbuf = gdk_pixbuf_new_from_file("images/felt.png", &error);
	boxFeldPixbuf = gdk_pixbuf_scale_simple(boxFeldPixbuf, 790, 160, GDK_INTERP_BILINEAR); 	
	boxFeltImagePlayer = gtk_image_new_from_pixbuf(boxFeldPixbuf);
	gtk_fixed_put(GTK_FIXED(frame), boxFeltImagePlayer, 5, 315);
	boxFeltImageDealer = gtk_image_new_from_pixbuf(boxFeldPixbuf);
	gtk_fixed_put(GTK_FIXED(frame), boxFeltImageDealer, 5, 5);


	//INIT PLAYER SCORE LABEL
	totalPlayerLabel = gtk_label_new("");
	gtk_label_set_markup (GTK_LABEL (totalPlayerLabel), "<span foreground=\"#3d3d3d\"> <big><b>Player Points: 0</b></big></span>");
	gtk_fixed_put(GTK_FIXED(frame), totalPlayerLabel, 15, 270);	
	//INIT PLAYER WINS LABEL
	totalWinsPlayerLabel = gtk_label_new("Player Wins: 0");
	gtk_label_set_markup (GTK_LABEL (totalWinsPlayerLabel), "<span foreground=\"#3d3d3d\"> <big><b>Player Wins: 0</b></big></span>");
	gtk_fixed_put(GTK_FIXED(frame), totalWinsPlayerLabel, 180, 270);	

	//INIT DEALER SCORE LABEL
	totalDealerLabel = gtk_label_new("");
	gtk_fixed_put(GTK_FIXED(frame), totalDealerLabel, 15,190);
	//INIT DEALER WINS LABEL
	totalWinsDealerLabel = gtk_label_new("");
	gtk_label_set_markup (GTK_LABEL (totalWinsDealerLabel), "<span foreground=\"#3d3d3d\"> <big><b>Dealer Wins: 0</b></big></span>");
	gtk_fixed_put(GTK_FIXED(frame), totalWinsDealerLabel, 180, 190);

	//INIT GAME STATE LABEL (win/lose/tie
	stateLabel = gtk_label_new("");
	gtk_fixed_put(GTK_FIXED(frame), stateLabel, 160, 180);


	//HORIZONTAL BOX FOR PLAYER TO ADD CARDS TO
	playerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
	gtk_fixed_put(GTK_FIXED(frame),playerBox, 20, 320);

	//HORIZONTAL BOX FOR DEALER TO ADD CARDS TO
	dealerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
	gtk_fixed_put(GTK_FIXED(frame),dealerBox, 20, 10);


	//DRAW CARD BUTTON	
	GtkImage *drawButtonImage = (GtkImage *) gtk_image_new_from_file("images/cardicon_s.png");
	drawCardButton = gtk_button_new_with_label("Draw Card");
	gtk_button_set_image((GtkButton *) drawCardButton, (GtkWidget *) drawButtonImage);	
	gtk_widget_set_size_request(drawCardButton, 70, 20);
	gtk_fixed_put(GTK_FIXED(frame), drawCardButton, 15, 215);
	//add actions to the button
	g_signal_connect(drawCardButton, "clicked", G_CALLBACK(drawCard), playerBox); //when clicked, draw a card and pass the player's box
	g_signal_connect(drawCardButton, "clicked", G_CALLBACK(updatePoints), totalPlayerLabel); //when clicked, update points and pass player's points label
	g_signal_connect(drawCardButton, "clicked", G_CALLBACK(checkAndDisable), NULL); //when clicked, check if player lost -> set game state accordingly and hide draw card button
	g_signal_connect(drawCardButton, "clicked", G_CALLBACK(checkGameState), stateLabel); //check game state and act accordingly




	//STAY BUTTON
	GtkImage *drawStopImage = (GtkImage *) gtk_image_new_from_file("images/stop.png");
	stayButton = gtk_button_new_with_label("Stay");
	gtk_button_set_image((GtkButton *) stayButton, (GtkWidget *) drawStopImage);
	gtk_widget_set_size_request(stayButton, 50, 20);
	gtk_fixed_put(GTK_FIXED(frame), stayButton, 185, 215);
	g_signal_connect(stayButton, "clicked", G_CALLBACK(dealersTurn), NULL); //when the player stays, it's the dealer's turn -> start his play function
	g_signal_connect(stayButton, "clicked", G_CALLBACK(disableStayButton), NULL); //disable the stay button
	g_signal_connect(drawCardButton, "clicked", G_CALLBACK(checkAndDisable), stayButton); //disable the drawcard button
	g_signal_connect(stayButton, "clicked", G_CALLBACK(checkGameState), stateLabel); //check game state and act accordingly

	//RESTART BUTTON
	GtkWidget *restartButton;
	restartButton = gtk_button_new_with_mnemonic("_Restart");
	gtk_widget_set_size_request(restartButton, 50, 20);
	gtk_fixed_put(GTK_FIXED(frame), restartButton, 700, 225);
	g_signal_connect(restartButton, "clicked", G_CALLBACK(destroy),NULL); //remove everything from the player and dealer box (all cards)
	g_signal_connect(restartButton, "clicked", G_CALLBACK(updatePoints), totalPlayerLabel); // reset player points label to 0
	g_signal_connect(restartButton, "clicked", G_CALLBACK(enableButton), drawCardButton); //enable draw card button
	g_signal_connect(restartButton, "clicked", G_CALLBACK(enableButton), stayButton); // enable stay button
	g_signal_connect(restartButton, "clicked", G_CALLBACK(checkGameState), stateLabel); //remove text from state label
	
	
	


	//CLOSE MAIN WINDOW ON X
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);
	

	reset();	

	//END MAIN LOOP
	gtk_main();
	

	return 0;
		
}
