
  ;Achtung beim Brennen: Der Sprut-Brenner 5 erkennt den 16F628A manchmal nicht. Das liegt an der Codeprotection
	;Die entfernt der Brenner durch etwas geringe Spannung nicht zuverlässig.
		;Wenn garnichts mehr geht: "Paten-Chip" in die DIL-Fassung einsetzen also einen unbenutzten jedenfalls korrekt gelöschten 16F628
		; Dann die Erkennung durchführen. Dann "heimlich" den Paten-Chip  gegen den ICSP  Stecker austauschen und 5 bis 10 mal die Code Protection entfernen.
		;Danach gehts dann wieder zu programmieren.
	
 LIST P=PIC16F628A 
 #include P16F628A.inc

__config B'11110100010000' 


  ;EINSTELLUNGEN						EINSTELLUNGEN						EINSTELLUNGEN
  ;EINSTELLUNGEN						EINSTELLUNGEN						EINSTELLUNGEN
  ;EINSTELLUNGEN						EINSTELLUNGEN						EINSTELLUNGEN
  ;EINSTELLUNGEN						EINSTELLUNGEN						EINSTELLUNGEN
		;Variablen:


;Achtung:!!!  Wenn hier von Sekunden die Rede ist, dann handelt es sich um ein Zeitintervall, das im Laufe der Programmierung 
			; immer länger wurde, je mehr Befehle  und Interrupts dazukamen.  Man könnte die 200ms Zeitschleife verkürzen um zu korrigieren,
			;das würde aber auch andere Einstellungen beeinflußen. Deshalb lasse ich es so. Der Einschleichtimer geht nach korrekter Zeit (nur 1Prozent ungenau)

							;        1 sec  =    fast  2 sec
							;        1 sec  =    fast  2 sec   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
							;        1 sec  =    fast  2 sec    !!!!!!!!!!!!!!!!!!!!!!!!
							;        1 sec  =    fast  2 sec
		


			;  empfohlene Einstellungen mit drei Ausrufezeichen gekennzeichnet. Die anderen lieber nicht verändern. 
 ;												!!!



 ;Spannungswandler für Betriebsspannung:
PWMPeriode  EQU D'11' ;!!!	;hier PWM-Periodendauer einstellen um bei höherer UB den Wirkungsgrad etwas zu heben, 
								;oder Regelschwingungen durch zuviel Power-Angebot zu verhindern. (Knattern bei höherer UB)
							;mit 7 läufts  ab 2,8V  mit Periode 11 erst ab 3,3 . Darf nicht kleiner als 7, sonst Kurzschluß = Hardwarebeschädigung !!!!!!!!!!!
 ;betr. Brummen				; Werte über 80 kommen in den Tonfrequenzbereich runter. Über 20bis 30 dürfte nicht benötigt werden.
							; Die erzielbare Leistungsersparnis oberhalb von PR=11 ist sowieso unter  10Prozent. 
  

	;Lautstärkeregelung und Ausschaltung 
VRLevel1	EQU  B'11100001'       ;die vier Grenzwerte für die NF-Pegelmessung  standardmäßig 1/24, 2/24, 4/24, 8/24 von VDD (max möglich 15/24)
VRLevel2	EQU  B'11100010'			; VDD in diesem Gerät = 4,8V Die Spannungen werden in VRCON eingetragen und von der Spannugsreferenz erzeugt und mit
VRLevel3	EQU  B'11100100'			;Komparator verglichen Pegel 1 ist dann unter Level 1 und Pegel 5 über Level 4
VRLevel4	EQU  B'11101000'			; die drei Einsen vorne dran müssen natürlich bleiben, damit macht VRCON was anderes.
										;normalerweise würde ich an den Leveln nicht ändern, die sind schon ziemlich sophisticated
										;vor allem hängt fast alles andere dann mit drin.

	;Lautstärkeregelung:
SekProUe    EQU  D'2' ;!!! ;Anzahl der Sekunden pro denen eine Ueberst bei vorhandenem Signal erlaubt wird (will man absolut garkeine Übersteuerungen,
							; dann wird dadurch die Modulationslautstärke so gering, dass man mehr Verständlichkeit verliert, als gewinnt.) 3sec ist hier Standardwert.
							;Also senken wenn man's lauter will heben wenn man weniger Übersteuerungen will.
							
UeGeduldEnde  EQU  D'80'      ;Anzahl der Übersteuerungen nach denen Ueauswert beschließt,die Regelzahlen, dh die langfristigen Normallautstärkevergleichszahlen 
								;(Regelzahlen werden andernorts verglichen mit Klassensumme) lieber etwas zu senken.Es wird auf und ab gezählt.Null ist die 60
MehrUeWagen  EQU  D'41'		;Anzahl +1 der erlaubten aber ausgebliebenen Übersteuerungen nach denen Ueauswert beschließt,die Regelzahlen,  
								;dh die Normallautstärkevergleichszahlen(Regelzahlen werden andernorts verglichen mit Klassensumme) lieber etwas zu heben.
									;Es wird auf und ab gezählt.Neutralstellung ist die 60
						;KLASSENSUMMen liegen zwischen 65 und 80  (vorbehaltlich inc Vergabe für Üst) für eine gute Vollaussteuerung mit Sprache u.ä. schätze ich Summe = 72 als Idealwert.(ohne Sprechpausen)
        				; Die Klassensumme ist sozusagen der über eine Sekunde integrierte Lautstärke-Wert falls nicht gleich wegen Überst abgebrochen wurde.
						;Entfernt sich der obere Wert weiter von 60 ist die Schaltung etwas geduldiger, bevor sie aber letztenendes die Normallautstärke 
						;genauso senkt.Die Untergrenze wirkt umgekehrt, und muß mindestens 10 sein.(geschätzt)(Unterlaufvermeidung)
						;eigentlich machen die beiden Variablen nicht viel. Am besten lassen
						;Die Klassensumme drückt die aktuelle Modulationsstärke aus und wird entsprechend der Regelzahl eingestellt, indem die VU vergrößert oder 
						;verkleinert wird. Bei vollem VU Regel-Umfang hat sie also mit der am Empfänger gehörten Lautstärke zu tun, aber wenig mit der Lautstärke am Abhörort.


RegelZOG    EQU D'73';!!!   ;Regelzahl Obergrenze Standard 74  Begrenzt die langfristige automatische Modulationsstärke-Regelung die sonst der eingestellten Übersterungs
						;Häufigkeit folgt. Standard ist 74   (Versucht dann also die Modulationsstärke aufzudrehen, wenn weniger Übersteuerungen sind als 
						;erlaubt, indem die Zielmarge für die Klassenzahl heraufgesetzt wird. Aber bei 74 ist dann eben Schluß.) Setzt man diese Zahl auf 
						; einen niedrigen Wert z.B.71 hat man garantiert immer eine so niedrige Modulation, dass auch z.B. Trommelschläge (kurz und wesentlich lauter als Gespräch u.ä.)
						; sauber vor dem Hintergrund gehört werden. Dafür muß man aber die Ohren spitzen um die Hintergrundgespräche zu hören.

RegelZUG	EQU D'71';!!!	;Regelzahl Untergrenze  Wenn es einen nervt, dass die Modulation auf Dauer immer leiser wird, dadnn kann man entweder SEKproUe
						; senken also mehr Ü's erlauben, oder ganz rabiat hier die Untergrenze für die Regelzahl anheben.
						;die Untergrenze sollte nicht größer sein als die Obergrenze.
					
						;macht man Unter und Obergrenze der Regelzahl gleich, also meinetwegen beide gleich 72, dann  versucht sich die Modulationsstärke -bei Signal-
						;immer auf diesen Wert einzuregeln. Je nach Geräuschart, kann es bischen leise werden, oder übersteuern, dafür weiß man aber ,
						; was man kriegt.
						;Ganz rabiat wäre dann die Festlegung der VU-Stufe selber mit VUMinBegr und VUMaxBegr s.u. dann passt sich garnichts mehr an.

PrimstartRegelZahl  EQU D'72' ;!!! ;Mit dieser Regelzahl startet das Gerät beim ersten Einschalten nach dem Batterieanschließen.
								;Sinnvollerweise innerhalb Ober oder Untergrenze weil sonst nach einer Sekunde sowieso unwirksam.


SprechPaZ7     EQU D'7'   ;Sprechpausenabwarte-Zähler Erst nach sovielen (Standard 7) Sekunden ganz leise(nur unterste Klasse, einmal zweite K-Summe unter 67) wird das Leise-sein
							; als nicht vorhandensein oder ganz leise sein von Signal gewertet und die Verstärkung trotzdem aufgedreht. Und Abschalten überhaupt ermöglicht.
							;sonst ist ein gewisses Minimum an Lautstärke und L-Varianz erforderlich, damit die Regelung überhaupt reagiert. (Aber auch abwärts)
SprechPaZ11    EQU D'11'  ;Sprechpausenabwarte-Zähler Nach sovielen (Standard 11) Sekunden ziemlich leise keine Pegel größer als zwei erfolgt alternativ dasselbe,
							; wie bei Zähler 7
							;Wenn einen stört, dass jedesmal wenn mal einer nen Moment nichts sagt, die VU raufgeht und das erste Wort vom Satz 
							;furchtbar klirrt,kann man die Zahlen etwas erhöhen.
							;Stört einen aber, dass wenn einer was gesagt hat, es furchtbart lange dauert, bis die VU soweit aufgedreht ist, dass man 
							;die leise Antwort verstehen kann, kann man die Zahlen auch etwas senken.
							;Durchaus relevante Einstellung.

VUMinBegr   EQU		D'60';!!!;Werte von 60 bis 66 (beide inclusive ) für die beiden Begrenzungen sind möglich. Darüber und darunter keine Wirkung 								
VUMaxBegr   EQU		D'66';!!! ;schlimmstenfalls Absturz (nicht überprüft)  Legt man beide Werte auf 60, ist die Verstärkung minimal und das Gerät kann
							; auch von großem Krach nicht übersteuert werden, egal, was die Regelung gerne machen würde. Leises ist dann nicht mehr 
						;zu hören.VUMaxBegr darf nicht kleiner sein als VUmin Begr. Im Intervall zwischen beiden Werten entscheidet die VURegelung.
						; stellt man beide Werte auf 66 gibt das Gerät alle Flüstergeräusche perfekt wieder ohne eines zu verpassen, übersteuert aber gnadenlos
						; bei auch nur etwas größeren Lautstärken. Durch Übersteuerungen kann der Funk-Störpegel etwas ansteigen.
						;stellt man Min auf 60 und Max auf 66 hat man vollen Regelumfang, die Anpassung dauert aber eventuell einige Sekunden.
						; Max auf 66 stellen darf man ev. nur in einem sehr leisen Raum (Keller) weil die Lautstärkeauswertung 
						;(soweit man es auf sie ankommen lässt)sonst das Ausschalten nicht mehr erlaubt da ständig "übertragenswertes" Flöhe-Husten gehört wird.
						;Äußerst relevante Einstellung.
						;für 7 VU-Stufen 0bis 6 entspr 6 Widerständen an 6 Ports und einmal kein Port
						;niedrigste vVUistStufe = niedrigste VU  =Krachtolerant
						;Stufe 66 ist sehr empfindlich, man hört sogar das leise Ticken einer 2 Meter entfernten Wand-uhr.

		;Ausschaltung:
		;Lichtbewertung:
LichtBewNenn   EQU  D'2' ;!!! ;Alle soviel Sekunden fließt die Beleuchtungsauswertung  in den Ausschalt-Auf/Ab-Zähler ein. Als Bruchnenner der Ausschaltgewichtung sozusagen.
							;Hier lasssen sich verschiedene Verhältnisse einstellen um das gewünschte timing einzustellen. Standard ist 3
							; Bruchztähler sind sozusagen die folgenden Bonusse und Malusse (Ich weiß, man schreibt das eigentlich mit Apostroph und ohne Pluralendung):
							;Der Nenner gilt auch für die Kompensation

LichtanBonus   EQU  D'5' ;!!!  ;Um soviel wird der Ausschaltzähler heraufgezählt wenn Licht an ist also die Einschaltdauer verlängert.
							;(das findet statt bei jedem LichtBewNenn =0 also standardmäßig alle drei sec)
							;trägt man hier 4 ein, schaltet sich das Ding wohl garnicht mehr aus, solange es hell ist.(Helligkeitsschwelle =ca Kellerfunzel indirekt gilt grade noch als hell)

LichtanMalus   EQU  D'0' ;!!!   ;Um soviel wird der Ausschaltzähler heruntergezählt wenn Licht an ist also die Einschaltdauer verkürzt.
							;Falls das Ding schreckhaft sein soll, um bei Licht nicht angepeilt werden zu können, oder nur das Dunkelgeflüster 
							;interessant genug ist um dafür Batteriestrom zu vrschwenden.

LichtKompensMalus  EQU  D'3';!!! ;hiermit kann man eigentlich alles kompensieren, egal,warum es nicht schnell genug ausschaltet. Nicht nur Licht.
							 ;Wenn man alle drei (oder n)Sekunden bei Licht  den Ausschaltzähler heraufzählt, empfiehlt es sich, ihn kompensatorisch 
							;in gleichen Intervallen herunter zu zählen, damit im Dunkeln das Ausschaltverhalten gleich bleibt, und nur im Hellen länger an.
							;vergrößert man den Malus, schaltet es schneller aus.

LichtKompensBonus  EQU  D'0';!!!  ; hiermit kann man eigentlich alles kompensieren.Bonus vergrößern =es bleibt länger an.  Entweder Bonus oder Malus sollte gleich Null.
								;alles andere ist albern, weil ein Bonus hier  genau einen Malus hier kompensiert.
			;Das obige gilt umgekehrt für Liebhaber der Dunkelheit: soviel wie hier drin steht, wird alle n Sekunden heraufgezählt= Ein-Zeit verlängert.
					;Frustauswertung für die Ausschaltung :
					;Frust-Steuerung: Wenn die Maschine selbst ein Frustrationsverhalten zeigt, werden dem Zuhörer Frustrationen erspart.
FrustBerückNenn    EQU  D'3';!!!  ;(Standard 3)Alle soviel Sekunden fließt die Frustberücksichtigung In die Ausschaltbewertung ein. Das heißt, wenn eine Frustsituation
								;festgestellt wurde, wird der Ausschaltzähler jedesmal noch eins extra runtergezählt, oder sogar zwei.
							;Eine Frustsituation ist, wenn sich das Gerät schon oft eingeschaltet hat, ohne dass hohe oder stark wechselnde Pegel gemessen 
							;wurden.Das regelt ein langfristiger Auf/Ab-Zähler. D.h. beim Empfänger geht der Squelch auf und dann ist nichts lohnendes zu hören
			;Achtung: Nicht auf einen zu hohen Wert stellen, weil auch die Anpassung der Vox - Empfindlichkeit bei dieser Gelegenheit erfolgt.
			; (Mindestens einmal pro Einschaltung.)
			;Denn es ist natürlich auch frustrierend, wenn das Ding dauernd umsonst anspringt, selbst wenn es sich dank anderer Einstellungen vielleicht schnell wieder ausschaltet.
FrustBerueckZaehler  EQU  D'1';!!!;(Standard 1)Der entsprechende Zähler. ( je nach Vox-Stufe wird er zweimal einmal oder garnicht vergeben.)
							;Wenn Frust fetsgestellt wurde, oder eine hohe Voxstufe festgelegt, schaltet es sich dadurch schneller aus.
							; Wieviel schneller, entscheidet diese Zahl. Große Zahl = in dem Fall noch schneller aus.


		;weiter mit Ausschaltung:
	;Berücksichtigung der absoluten Lautstärke für die Ausschaltung:


AbsolautNenn   EQU  D'3';!!!   ;Alle soviel Sekunden werden Bonus'  für absolute Lautstärke Pegel vergeben. 
                            ;(Diese Bewertung konkurriert mit der Bewertung für mehrfachen Lautstärkewechsel innerhalb einer Sekunde (der Klassenzahl))
							; Will man den "nervigen Quatsch" mit der Berücksichtigung sprachähnlicher Lautsärkerythmen nicht, dann wählt man hier einen 
							;kleineren Nenner mindestens =1  für die Bonusse größeren Zähler.
AbsolautLo    EQU   D'67';!!!  ; Für Klassensummen  (also Lautstärkebewertende Zahlen zwischen 65 und80) wenn größer/= dieser Zahl, (Standard 68) werden 
AbsoLoBonus   EQU   D'3' ;!!!   ; soviele Bonus vergeben, wie -hier- steht. Das heißt, wenn's bei Geräuschen länger an bleiben soll,
								 ;muß man hier (untere Zeile) wenigstens "1" reinschreiben

							
				;Wenn man absolaut stark oder ausschließlich berücksichtigt, dann bedeuten niedrige Grenzwerte, dass es nur bei sehr leise ausschaltet.
				;hohe, dasses nur bei sehr laut anbleibt.
AbsolautHi    EQU   D'71';!!!  ; Für Klassensummen  (also Lautstärkebewertende Zahlen zwischen 65 und80) über dieser Zahl, (Standard 72) werden
AbsoHiBonus   EQU   D'3' ;!!!  ; soviele zusätzliche Bonus vergeben, wie -hier- steht. Also einer für über 68 und noch einer für über 72.
							;wenn's bei Krach länger an bleiben soll, muß man hier wenigstens "1" reinschreiben
							;Die Absolaut-Bewertung ist aber relativ zur Lautstärkeregelung. Solange noch die Verstärkung hoch ist, 
							;werden hier auch leisere Geräusche bewertet.Aber gegen Ende wird ja die VU durch die Lautstärkeregelung aufgedreht, soweit erlaubt.
							;Und wenn dann immer noch keine hohen Pegel sind, dann verlängert auch Absolaut nicht mehr die Einschaltzeit.
							;Will man Absolaut verwenden (Bei anderen Geräten das einzige was es gibt) muß man also uU die Maximalverstärkug begrenzen s.o.

UestVerlaengerung  EQU D'1'  ; wenn es bei Übersteuerungen grade zu schnell abschaltet oder länger laufen soll.



;Auswertung der Klassenzahl also der Lautstärkevarianz innerhalb einer Sekunde für die Ausschaltung:
EinklassenMalus  EQU D'2';!!!  ; Wenn bei den fünf Messungen in der ganzen Sekunde nur eine Lautstärkeklasse vorgekommen ist(also immer dieselbe) spricht das
							;dafür, dass ein Störgeräusch vorliegt (irgendwas brummt oder rauscht) Das spricht eher für ausschalten Also wird der Auf/Ab
							;Zähler für die Ausschaltung soviel runtergezählt wie hier steht.  (Standard =1)
							; Achtung, die Zahlen fallen mehr ins Gewicht, weil die Auswertung jede Sekunde erfolgt.
							  ; Bei Zwei Klassen passiert garnichts
DreiklassenBonus EQU D'4' ;!!! ;Bei Drei Klassen was selten ist, aber stark für interessante Geräusche z.B.Sprache spricht, gibt es Bonusse (Standard=2)

VierklassenBonus EQU D'6' ;!!! ;Bei Vier Klassen (echt ziemlich selten, aber interessant) standardmäßig drei Bonusse.(Addiert sich nicht mit dem dreiKlassen Bonus.Entweder oder.Also keine fünf zusammen)
							;Fünf Klassen kommen nicht vor, da die fünfte Klasse als Übersteuerung gewertet wird und sofort VU runterschaltet und deshalb nicht in der Klassensumme enthalten sein kann.
		
		;Grenzen des Ausschalt-Bonus (also Geduld sozusagen)
OfenausZahl   EQU D'40'    ; (Standard 40) Unterschreitet der AusschaltBonus bei sekündlicher Auswertung diese Zahl, dann wird das Gerät ausgeschaltet. (Sofort und überhaupt)
							;Die Zahl sollte mindestend zwanzig betragen, da mehrere Herabsetzungen des Ausschaltbonus durch verschiedene Module 
							;erfolgen können, ehe der Bonus ausgewertet und gedeckelt wird. Natürlich muß eine Unterschreitung von Null unbedingt vermieden werden.
OfenheissZahl  EQU D'70'   ;Der maximal mögliche Wert für  den Ausschaltbonus. Denn: auch wenn die Übertragung noch so schön:Wenn sie zuende ist, soll
							;es ja auch noch mal vor morgen ausschalten.  Standardmäßig 70  Muß unbedingt unter 230 (Das ist aber auch ne Ewigkeit)
							;Startwert für den AusschaltBonus (nach dem Aufwachen durch ein Geräusch) ist 60

StartBonus    EQU D'70'     ;StartBonus wird nach dem Erwachen als AusschaltBonus geladen. Dafür dass es sich überhaupt eingeschaltet hat, wird eine 
							;gewisse Zeit bedingungslos vorgegeben, ohne dass die Bewertungscharakteristik bei längerem Laufen dadurch verändert wird.
							;StartBonus muß ungefähr  über Ofenaus liegen und ungefähr unter Ofenheiss. Ein paar mehr oder weniger können noch Sinn
							; machen, also soviel wie bis zur ersten Bewertung nach 1 "Sekunde" geändert worden sein kann.Also normalerweise so +/- 4
							;Standard für StartBonus ist 60
Voxoffset   EQU   D'3' ;!!!   ;(Standard=1)  Voxoffset=0 d.h. immer unempfindlich beide empf. Stufen deaktiviert. Vermeidet Nerverei 
				;1=empfindlichste Stufe deaktiviert.(Standard.Geht in normalen Räumen kaum anders da die Weckschaltung wirklich flüsterempfindlich ist.)							
 					;2= voller Regelumfang alle drei Stufen möglich. Nur wo es wenigstens manchmal ganz ganz leise ist.
							; 3= unempfindlichste Stufe deaktiviert. 
					;4= beide unempfindlichen Stufen deaktiviert.Um immer jedes Flüstergeräusch mitzukriegen.Geht aber jedesmal los, wenn in der 
						;Wohnung drüber ein Kind rumrennt,oder was vom Tisch fällt.
				;Der Voxoffset hat keinen Einfluß auf vVoxist Sufe, (den von der Regelung gewünschten Wert) 
				;weil er erst danach wirkt und diese nicht verstellt.Die Ausschaltbeschleunigung durch Frust bleibt also unbeeinflußt.



MeldeIntervall  EQU  D'27';!!!  ; Alle soviel Sekunden wird, --- falls --- das Gerät grade sendet, ein Meldeton gesendet, der Auskunft gibt, 
					    	;ob es im überwachten Raum hell ist.
							; Null heißt, es werden keine Meldetöne gesendet.  =  Feature deaktiviert.    
							; Ein BIP-Ton bedeutet, es ist hell,  zwei Töne: es ist dunkel. Merken:dunkel = zweisilbig. Hell = einsilbig.
							; Falls das Feature überhaupt aktiviert ist, kommt der erste Ton 4 sec nach  Erwachen. 254 = alle 8 Minuten weitere Töne.
							;Meldetöne haben ein etwas größeres Störspektrum und etwas größere Reichweite, darum nicht zu kurze Intervalle wählen!
ErstMeldung		EQU  D'4'   ;soviele  "Sekunden" nach dem Erwachen wird der erste Meldeton gesendet. (Falls überhaupt.)  4 "Sek" sind ungefähr 7 Sekunden.
							;nicht zu schnell nach dem Einschalten, da der Lichtsensor sehr hochohmig ist und mehrere Sekunden Anpasszeit hat.

Einschleichzeit EQU D'10';!!!  ;(Standard = 0h bzw.Trottelsperre 10min) Erst nach soviel  Minuten/Stunden wie hier steht macht sich das Gerät sendebereit. bis dahin wartet es relativ stromsparend ohne Aktion
							;ist also bis dahin auch nicht ortbar.0 bis 254

Einschleichmassstab  EQU D'1';!!! ; 1 heißt hier, dass die Einschleichzeit in Minuten gemessen wird also bis 4Stunden ca. 2heißt in Stunden also bis ca 10 Tage.
								;ACHTUNG ACHTUNG:  Andere Werte als 1 oder zwei sind hier nicht vorgesehen und führen zum Absturz!
								;Langfristigere Verzögerungen z.B. 254 Tage machen sowieso keinen Spass. Und bei noch längeren ist sowieso die Batterie leer.



 ;RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR

												;REGISTER-LISTE:
												;REGISTER-LISTE:
												;REGISTER-LISTE:
												;REGISTER-LISTE:
	;Register:

vCopyRota EQU  0x22
vPWMCon  EQU  0x23
;XXXleer9 EQU  0x24 
;XXvRech1   EQU  0x25 
vPegel1  EQU  0x26
vPegel2  EQU  0x27
vPegel3  EQU	0x28
vPegel4  EQU	0x29
vPegel5  EQU	0x2A
    ;Zeitschleife
Kor1MReg   	EQU     0x2B		;plus Konstante-Register  für Schl1M
Inn1MReg   	EQU     0x2C		;innere Schleifenzahl mal
Auss1MReg	EQU     0x2D		;aeussere Schleifenzahl-Reg
SupAuss1M   EQU		0x2E		;ganz äussere SchlZahl-Register

Schl20Reg  EQU   0x2F

vMyFlags  EQU   0x30   
						; MyFlags,0   Set = Ausschaltung nach sleep ist vorgesehen Abschluss einstellungen vornehmen
						; MyFlags,1 gesetzt: In letzter Sekunde waren nur 1oder2 Lautstärkeklassen
						; MyFlags,2  gesetzt: 1n letzter Sekunde war ein VU-Stufen-Wechsel
						; MyFlags,3 nur für Testzwecke in fertigem Programm unbenützt
						; MyFlags,4  RelevanzBit:gesetzt: Es wurde kürzlich eine Geräuschkulisse im Aussteuerungsbereich wahrgenommen
						; MyFlags,5  set =letzter Durchgang  war 3oder 4 Klassen  (ab Ausschaltbewertung)
						; MyFlags,6   ;Es wird bis zur nächsten Klassensumme gespeichert ob ein oder mehr Übersteuerungen vorkamen.
						; MyFlags,7   ;Es wird bis zur nächsten Klassensumme gespeichert ob ein oder mehr Übersteuerungen vorkamen.

					
v5Runden        EQU  0x31
Kor100Reg       EQU  0x32		;plus Konstante   für Schl100
Inn100Reg       EQU  0x33		;innere Schleifenzahl mal
Auss100Reg	    EQU  0x34		;aeussere Schleifenzahl
vAusschaltBonus EQU  0x35
vEreignis       EQU  0x36
vBeschleuniger   EQU  0x37
vKlassenZahl    EQU  0x38
;XXXleer6		EQU  0x39
;XXXleer7       EQU  0x3A
vUebersteuerung EQU  0x3B
vZaehl60  		EQU  0x3C
vVoreinstellVU  EQU  0x3D
vVUistStufe     EQU  0x3E
vVUistBank1		EQU   0x74         ; Register in Bank 1 damit ich nicht dauernd wechseln muß beim VUports Trisa-Stellen geändert:Allbank
vVoxIstStufe    EQU   0x72      ;Allbankadresse
WTemp           EQU   0x70     ; Als Interrupt Sicherungsregister in allen Bänken addressierbar.(Bereich 70 bis7F)
StatusTemp      EQU   0x71      ; StatusTemp dürfte aber überall in Bank 0 liegen da nur von dort aus addressiert
;xxxleer4		EQU  0x3F
vKlassenSumme   EQU  0x40
vRegelZahl      EQU  0x41
vZähler7		EQU  0x42
vZähler11    	EQU  0x43
vRegelZahlSum   EQU  0x44
vUeErlaubt      EQU  0x45
vFrust  		EQU  0x46
vAbsoZaehl      EQU  0x47
vFrustBerücksi  EQU  0x48
vInn			EQU  0x49
vAuss			EQU  0x4A
vSupAuss		EQU  0x4B
vXh3			EQU  0x4C
vStart1			EQU  0x4D
vMeldeIntervall EQU  0x4E
vTonADauer		EQU  0x4F
vTonAFreq		EQU  0x50
vMyFlagsB       EQU  0x73
vLimitCounterA  EQU  0x51
vLimitCounterB  EQU  0x52
vFeierSignal    EQU  0x53   
vProoftemp1	    EQU  0xA4		;RAM Adr    BANK1 für EEPROM-Bearbeitung
vProoftemp2	    EQU  0xA5		;RAM Adr    BANK1 für EEPROM-Bearbeitung	
vLoBysav    	EQU  0xA6		;RAM Adr    BANK1 für EEPROM-Bearbeitung		
vUpBysav   	    EQU  0xA7		;RAM Adr    BANK1 für EEPROM-Bearbeitung


  ;EEPROM-Adressen:

eInitproof1	 EQU    0x01        ;EEprom Adresse
eInitproof2  EQU    0x02		;EEPROM Adr
eUpEEP		 EQU	0x03		;EEPROM Adr
eLoEEP    	 EQU    0x04		;EEPROM Adr



			; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
				; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
					; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
						; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
							; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
								; HAUPT PROGRAMM  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


	CALL Startverzögerung
	CALL PrimInit
Ruf1 CALL WInit
	GOTO Maerchen
Vector4 NOP
    BTFSS INTCON,2    ;Interrupt-Check (TMR0-Flag betr Nachregelungsroutine PWM Spannungswandler)
	GOTO  Ruf0
	GOTO PWMStart
	NOP

    ;Schlafvorbereitung
Maerchen BSF   STATUS,RP0      ;PWM aus
	BCF  VRCON,7
	BSF  TRISB,0
	BCF STATUS,RP0
	MOVLW   B'00000111'
	MOVWF   CMCON
    BCF INTCON,5         ;timer0 disablen (PWM Stellzyklus-Timer)
	CLRF CCP1CON       ;PWM aus mit output low
	BCF T2CON,2
	BCF PORTA,3    ;Sendestufe ausschalten
	BCF PORTB,3			;da P6 jetzt normaler inout-Pin den lieber auch nochmal low, sonst ist Der PWM-SchaltFet im Kurzschluss
	CALL Schl1M   ;Päuschen, damit garantiert Ruhe an der Vox eintritt (ev verkürzen)

	BCF INTCON,1      ;RB0 Interrupt scharf machen FLAG
	BSF INTCON,4		;enable
	BSF INTCON,7		; GIE
	SLEEP
Stop	NOP
	;	GOTO  Stop       ; Nur zum Testen damit's nicht gleich wieder einschläft muß wieder deaktiviert !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	GOTO Maerchen
	NOP
;-----------------------------------

Ruf0 BTFSC INTCON,4  ;Interrupt-Check  RB0 (Vox aufwachen) enable. Dann kann es ja nur noch ein Aufwachbefehl sein.
	GOTO Ruf2
	BCF INTCON,1      ;RB0 FLAG zurück darf eigentlich garnicht vorkommen
	RETFIE

							;WACH		   WACH			   WACH
							  ;WACH		  WACH	WACH     WACH
							    ;WACH	WACH	  WACH WACH
							      ;WACH			    WACH			


        ;LAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXXLAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXXLAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXX
 ;LAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXXLAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXXLAUTSÄRKEBEDINGTE EINSTELLUNGEN 	-----XXX

 ; 1.LautstärkeErfassung mit Comparator
    
        ;grade aufgewacht
Ruf2	CLRF  PORTB
		CALL WInit		;durch das Schlafen wurden die Einstellungen für PWM zerstört.Also nochmal
		BCF STATUS,RP0     ;sicher ist sicher
		MOVF vVoreinstellVU,W			;erst mal Verstärkungsfaktor auf gespeicherten Wert voreinstellen
		MOVWF vVUistStufe
		CALL VUAusgabe	
		CALL Schl200                     ;grade aufgewacht erst mal warten, bis sich Verstärker eingeregelt hat.
		MOVLW StartBonus				;alle Register vor Unterlauf schützen
		MOVWF    vAusschaltBonus 
		MOVWF   vUebersteuerung 
		MOVWF		vZaehl60
				MOVLW  D'1'     
				MOVWF  vAbsoZaehl
			BCF  vMyFlags,0              ;Nur bei Aufwachen clearen und bei Ausschalten setten
				BSF   PORTA,3      ;Die Sendestufe wird bei der Gelegenheit eingeschaltet YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY


	
				CLRW				; der Programmabschnitt sorgt dafür, dass ca 4 sec nach Anspringen der Erste Meldeton gesendet wird,falls gewählt.
				ADDLW MeldeIntervall  ;(Null-Prüfung)
				BTFSC STATUS,Z
				GOTO  Messen
				MOVLW  ErstMeldung    
				MOVWF  vMeldeIntervall

				


Messen	BCF STATUS,RP0         
	MOVLW B'00000101'
	 MOVWF  CMCON
	BSF STATUS,RP0
	BCF PIE1,6
	BSF TRISA,1		;ComparatorTrisa
	BSF TRISA,2
			BSF TRISA,4    ;Trisa für Lichtmessung einstellen

	MOVLW VRLevel1   ;Level 1/24           ;Level müßen ev praxisangeglichen werden OK. VDD= 4,8V
	MOVWF  VRCON			;ReferenzSpannung wird auf 1/24 VDD eingestellt, anschließend mit Spannung an Pin 18 verglichen
	BCF STATUS,RP0			;  War VPin18 niedriger  wird das Register für die Pegelstufe 1 incrementiert und die Messung damit erfolgreich abgebrochen
    CALL  zehnMue			; War V Pin 18 höher erfolgt kein Eintrag und es geht zum nächsten Test mit höherer VRef
	BTFSS CMCON,7
	GOTO  Messen2
    INCF vPegel1,F  ;Eintrag in Pegelstufe 1   unter1/24 xVdd  
	Goto  MessenFertig

Messen2	BSF STATUS,RP0
	MOVLW  VRLevel2    ;Level 2/24
	MOVWF  VRCON
	BCF STATUS,RP0
    CALL  zehnMue
	BTFSS CMCON,7
	GOTO  Messen3
    INCF vPegel2,F  ;Eintrag in Pegelstufe 2    1bis2  /24 xVdd
	Goto  MessenFertig

Messen3	BSF STATUS,RP0
	MOVLW  VRLevel3     ;Level 4/24
	MOVWF  VRCON
	BCF STATUS,RP0
    CALL  zehnMue
	BTFSS CMCON,7
	GOTO  Messen4
    INCF vPegel3,F  ;Eintrag in Pegelstufe 3  2bis4  /24 xVdd
	Goto  MessenFertig

Messen4	BSF STATUS,RP0
	MOVLW  VRLevel4     ;Level 8/24
	MOVWF  VRCON
	BCF STATUS,RP0
    CALL  zehnMue
	BTFSS CMCON,7
	GOTO  Messen5
    INCF vPegel4,F  ;Eintrag in Pegelstufe 4 = 4bis10  /24  xVdd
	Goto  MessenFertig

Messen5	incf vPegel5,F     ;Eintrag in Pegelstufe 5 für über 8/24  xVdd  reduziert auf 8

        MOVLW   D'1'       ;Instant - Reaktion auf Übersteuerungen  setzt ein ab zwei Ü's reduziertauf1
		SUBWF   vPegel5,W
		BTFSS   STATUS,C
		GOTO    MessenFertig  
		MOVLW   D'60'    		;wenns schon auf höchster Stufe ist, soll Dauerübersteuerung nicht das ganze sekündliche Programm zum Erliegen bringen.
		SUBWF    vVUistBank1,W
		BTFSC    STATUS,Z
		GOTO    MessenFertig
		DECF    vVUistStufe,F    ;sofortige VU-Stellung (???) -eigentlich bewährt-.
		CLRF v5Runden				;wenn ich die Stufe runtersetze messe ich sinvollerweise anschließend neu
		 		MOVLW UestVerlaengerung   ;Da bei Üst Bonus' ausfallen,kann hier ausgeglichen werden.
				ADDWF  vAusschaltBonus,F
		BTFSC   vMyFlags,6			;Es wird bis zur nächsten Klassensumme gespeichert ob ein oder mehr Übersteuerungen vorkamen.
		BSF     vMyFlags,7
        BSF     vMyFlags,6		
		CALL    UeAuswert
		GOTO    VUStell3a
		

	;Zahlen in den Pegelregistern vPegel 1-5 liegen zwischen 0 und 5 je nachdem wie oft der betreffende Pegel bei 5 Messungen vorkam.Zusammen also immer 5.
	; Die Pegel entstammen der Gleichrichtung  der Signalspannung am Ausgang der Regelstufe.

     
			
MessenFertig	CALL Schl200	;diesmal war's offensichtlich keine Übersteuerung oder wir sind schon auf höchster Stufe.	; 1/5 sec Pause
			incf v5Runden,F		; Jede 5. Messrunde erfassen um einmal pro sec auszuwerten
			MOVLW D'5'
			SUBWF v5Runden,W
			BTFSS STATUS,C
			GOTO Messen
			CLRF v5Runden
			GOTO Auswertung
			

; Auswertung der Peg		el 	  nur alle		     5 Mess       ungen also ca 1 mal/sec        --------------------XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX------------------------------------------------
; Auswer		tung		der Pegel    n		   ur alle 5 Me     ssungen also ca 1 mal/sec 
; Auswer		tung 		der 				Pegel         nur        alle 5 Messungen also ca 1 mal/sec 
; Auswert	ung der 		Peg					l nur         alle      5 Messungen also ca 1 mal/sec 
; Auswertung der 			Pegel    nur a		lle 5                    essungen also ca 1 mal/sec 
; Auswer					tung der Pegel       nur al    le 5 Mess    ungen also ca 1 mal/sec 
; Auswer					tung 				  der P    egel    nu    r alle 5 Messungen also ca 1 mal/sec 
; Auswer					tung 				  der Pe        gel    nur alle 5 Messungen also ca 1 mal/sec 
; Auswer					tung der Pegel          nur al   le 5 M    essungen also ca 1 mal/sec 
; Auswer					tung der Pegel             nur alle 5      Messungen also ca 1 mal/sec 

 ;Ermittlung der KLASSENZAHL:

Auswertung  CLRF  vKlassenZahl     ;ermitteln wieviele verschiedene Lautstärkeklassen in der letzten Sekunde vorgekommen sind. steht dann in vKlassenZahl.
	  	MOVLW D'1'
		SUBWF  vPegel1,W
		BTFSC  STATUS,C
		INCF vKlassenZahl,F

	  	MOVLW D'1'
		SUBWF  vPegel2,W
		BTFSC  STATUS,C
		INCF vKlassenZahl,F

	  	MOVLW D'1'
		SUBWF  vPegel3,W
		BTFSC  STATUS,C
		INCF vKlassenZahl,F

	  	MOVLW D'1'
		SUBWF  vPegel4,W
		BTFSC  STATUS,C
		INCF vKlassenZahl,F
	
	  	MOVLW D'1'
		SUBWF  vPegel5,W
		BTFSC  STATUS,C
		INCF vKlassenZahl,F
		
			;Klassenzahlen liegen zwischen 1 und 5  voll ausgesteuerte Sprache liegt bei ca. 2-4  (ohne Sprechpausen)

  ;Ermittlung der KLASSENSUMME:

		MOVLW  D'60'
		MOVWF  vKlassenSumme       ; Vor-Ladung mit 60 zur Vermeidung von Carry-Problemen CLEART automatisch die Klassensumme 

		MOVF vPegel1,W
		ADDWF  vKlassenSumme,F     
		
		MOVF vPegel2,W
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F

		MOVF vPegel3,W
		ADDWF vKlassenSumme,F	
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F

		MOVF vPegel4,W
		ADDWF vKlassenSumme,F	
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F

		MOVF vPegel5,W          
		ADDWF vKlassenSumme,F	
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F
		ADDWF vKlassenSumme,F


		BTFSS vMyFlags,6   			;seit Pegel 5 eine Instant-Reaktion hervorruft (Änderung 4-9/5)
		GOTO  UeMuClr       			;kann er ja in der Klassensumme nicht mehr vorkommen.! geändert:Kann er wieder, aber nur bei höchster VU-Stufe.
		INCF  vKlassenSumme,F			;Zum Ausgleich können hier beim nächsten regulären Durchgang incs vergeben werden, um nach Übersteuerungen VU erstmal bischen unten zu halten.
		BTFSS vMyFlags,7				; seit in allen Versionen der 5-Messrundenzähler (oben)nach Üst gecleart wird aber ev. gar nicht mehr nötig.
		GOTO  UeMuClr
		INCF  vKlassenSumme,F		;  (sozusagen das D in der PID Regelung durch Uest)
UeMuClr	BCF   vMyFlags,7
		BCF   vMyFlags,6
		

		;KLASSENSUMMen liegen zwischen 65 und 80  (vorbehaltlich inc Vergabe für Üst) für eine gute Vollaussteuerung mit Sprache u.ä. rechne ich Summe = 72 als Idealwert.(ohne Sprechpausen)
        ; Die Klassensumme ist sozusagen der über eine Sekunde integrierte Lautstärke-Wert falls nicht gleich wegen Überst abgebrochen wurde.
  ; 3. ---VU- Regelung		---VU- Regelung---VU- Regelung---VU- Regelung------------XXXXXXXXXXXXXXXX----------------------------------------
  ; 3. ---VU- Regelung		---VU- Regelung---VU- Regelung---VU- Regelung------------XXXXXXXXXXXXXXXX----------------------------------------
  ; 3. ---VU- Regelung		---VU- Regelung---VU- Regelung---VU- Regelung------------XXXXXXXXXXXXXXXX----------------------------------------
  ; 3. ---VU- Regelung		---VU- Regelung---VU- Regelung---VU- Regelung------------XXXXXXXXXXXXXXXX----------------------------------------
  ; 3. ---VU- Regelung		---VU- Regelung---VU- Regelung---VU- Regelung------------XXXXXXXXXXXXXXXX----------------------------------------
	

		; Ermittlung der Regelzahlen aus der Übersteuerungshäufigkeit
				
				;regelmäßiges Decrementieren des Übersteuerungsspeichers  bei Signal:
				
Uedecr			MOVLW  D'79'
				SUBWF  vKlassenSumme,W
				BTFSC  STATUS,C
				Goto UeAuswert1
				MOVF    vRegelZahlSum,W
				SUBWF  vKlassenSumme,W
				BTFSS  STATUS,C
				Goto UeAuswert1
				DECFSZ vUeErlaubt,F   ;hier kommt man also hin bei Klassensummen zwischen vRegelzahlSum und 78  d.h. Signal
				GOTO  UeAuswert1
				DECF  vUebersteuerung,F    ; In diesem Register kummulieren sich die vorgekommenen Übersteuerungen - 
												;oder auch nicht. Für die Langzeitregelung
				MOVLW SekProUe      ;Anzahl der Sekunden in denen eine Ueberst bei vorhandenem Signal erlaubt wird !!!!!!!!
				MOVWF vUeErlaubt

				;Auswerten des Übersteuerugsspeichers - Stellen der Regelzahlen

UeAuswert1				CALL  UeAuswert




		;Relevanzüberprüfung der Messwerte anhand KlassenSumme

Relevanz	    MOVLW  D'67'				;MyFlags,4 Rücksetzroutine 7 mal Klassensumme unter 67?
				SUBWF	vKlassenSumme,W		;ohne MyFlags,4 beginnt eine unbedingte Verstärkungsstufensuche also Vu immer weiter erhöhen.	
				BTFSC   STATUS,C
				GOTO   Kl1u2
				DECFSZ  vZähler7,F
				GOTO   Kl1u2
				BCF    vMyFlags,4

Kl1u2			MOVF  vPegel5,W		;MyFlags,4 Rücksetzroutine:  alternativ 11 mal	keine Pegel größer als 2?
				BTFSS  STATUS,Z		;ohne MyFlags,4 beginnt eine unbedingte Verstärkungsstufensuche also Vu immer weiter erhöhen.
				GOTO  Relevanz1

				MOVF  vPegel4,W
				BTFSS  STATUS,Z
				GOTO  Relevanz1

				MOVF  vPegel3,W
				BTFSS  STATUS,Z
				GOTO  Relevanz1	
			
				DECFSZ  vZähler11,F
				GOTO   Relevanz1
				BCF    vMyFlags,4
				DECF  vUebersteuerung,F   ; gehört eigentlich ordentlicherweise nicht hierher, aber ich wollte für diesen Kompensationsmechanismus nicht schon wieder 
											;einen extra Zähler schreiben.Es könnte ohne den Befehl eine Art logischer latch down der VU auftreten.!!!!!!!!!

		;Relevanzüberprüfung der Messwerte anhand KlassenZahl:

Relevanz1		MOVLW D'2'						; Wenn Sprache ist und keine Sprechpause, dann wohl K-Zahl>=2 und  K-Summe >=70 dann wird MyFlags,4 gesetzt
				SUBWF  vKlassenZahl,W			;und die Sprechpausenzähler 7 und 11 zurückgesetzt. Wenn die Zähler von zu vielen Pausen auf Null 
				BTFSS  STATUS,C					;runter sind wird MyFlags,4 gecleart  und nicht nur "Sprach"signale  ausgewertet, sondern auch die niedrigen 
				GOTO   Relevanz2				; Level und die Klassenzahl 1 mit der Regelzahl verglichen. Es wird dann angenommen, dass die Lautstärke geringer geworden ist.
												;und die VU Stufe ohne Erhöhung keine auswertbaren Signale mehr gibt. (Mit vielleicht auch nicht.)

				MOVF    vRegelZahlSum,W ;also 70+/-2  nachgeregelt nach Übersteuerungshäufigkeit
				SUBWF	vKlassenSumme,W
				BTFSS   STATUS,C
				GOTO    Relevanz2

		        BSF   vMyFlags,4      ;Das "sprach"-Flag MyFlags,4 wird gesetzt

				INCF 	vAusschaltBonus,F
Relevanz3		MOVLW  SprechPaZ11			; Die Sprachpausen-Abwarte-Zähler werden wieder auf Startwerte gesetzt 
				MOVWF  vZähler11

				MOVLW  SprechPaZ7   		;dto.
				MOVWF  vZähler7
				GOTO Vergleicher

Relevanz2		BTFSC  vMyFlags,4		;wer hir ankommt wird nur ausgewertet, wenn in letzter Zeit keine "Sprache" erkannt wurde.
				GOTO   VUFertig

		;Regelung   (unter Verwendung der oben ermittelten Relevanz und Regelzahlen):

Vergleicher		INCF    vRegelZahl,W				;in einer Neutralzone zwischen Regelzahl +1 und Regelzahl -1 passiert keine Regeländerung
				SUBWF   vKlassenSumme,W						
				BTFSC   STATUS,Z
				GOTO  VUFertig	
				DECF    vRegelZahl,W
				SUBWF   vKlassenSumme,W						
				BTFSC   STATUS,Z
				GOTO  VUFertig			
				MOVF    vRegelZahl,W		;also 73+/-2 Vergleich von Regelzahl und Klassensumme um zu ermitteln ob VU gesenkt erhöht oder garnichts wird.
				SUBWF   vKlassenSumme,W						;Regelzahl nachgeregelt nach Übersteuerungshäufigkeit
				BTFSC   STATUS,Z
				GOTO  VUFertig
				BTFSC   STATUS,C
				GOTO    runter
rauf			INCF    vVUistStufe,F       ;HIER einzige Stell-Stelle!   Es gibt jetzt noch die Instant-Regelung oben Messen5
				GOTO    VUStell3a
runter			DECF    vVUistStufe,F		;HIER einzige Stell-Stelle!



		; Bei VUStell3a kommen die Sofort Calls von Übersteuerungen an. Bei Dauerübersteuerung werden die folgenden Befehle also in sehr schneller Folge bearbeitet.

VUStell3a		MOVLW  VUMinBegr				;Unterlaufschutz  VUist-Stufe   Vu ist-Stufe geht von inclusive 60 bis inclusive 66 !!!!!!!!
				SUBWF vVUistStufe,W             ;für 7 VU-Stufen 0bis 6 entspr 6 Widerständen an 6 Ports und einmal kein Port
				BTFSC STATUS,C						;niedrigste vVUistStufe = niedrigste VU
				Goto  VuStell4
				MOVLW VUMinBegr				
				MOVWF  vVUistStufe
				GOTO  VUFertig

VuStell4		MOVLW  VUMaxBegr				;Überlaufschutz  VUist-Stufe   VuistStufe geht von inclusive 60 bis inclusive 66
				ADDLW  D'1'
				SUBWF vVUistStufe,W              ;für 7 VU-Stufen 0bis 6 entspr 6 Widerständen an 6 Ports und einmal kein Port
				BTFSS STATUS,C
				Goto  VuStell5
				MOVLW VUMaxBegr
				MOVWF  vVUistStufe
				GOTO  VUFertig

VuStell5			CALL VUAusgabe     			;call zum Ports-Stellen



			BSF   vMyFlags,2   ;Das Vu-Stufen-ÄnderungsFlag
			GOTO Voreinstell			

VUFertig    BCF  vMyFlags,2


	;Abschließendes:
  ;-Voreinstellung VU nach Aufwachen  entsprechend den zuletzt erfolgreichen Werten.
	  

Voreinstell		BTFSC vMyFlags,2
				GOTO  ClearPeg

				BTFSS vMyFlags,4
				GOTO  ClearPeg

				BTFSS vMyFlags,5
				GOTO  ClearPeg

					; Wenn ich hier angekommen bin sind Klasse5 unbesetzt, und 3oder4 verschiedene besetzt es wurde Sprache erkannt und die Vu nicht gerade gewechselt
					; das sollte Grund sein, die VU- Einstellung als Starteinstellung zu speichern:
					MOVF vVUistStufe,W
					MOVWF vVoreinstellVU



;jetzt kommt erstmal alles restliche was einmal pro (nomineller) Sekunde stattfinden soll.
;-----------------Pegel clearen ------------
ClearPeg				CLRF vPegel1      ;die Pegelspeicher für die nächsten 5 Erfassungen clearen alle 5x200ms
				CLRF vPegel2
				CLRF vPegel3
				CLRF vPegel4
				CLRF vPegel5

; erst mal verhindern, dass bei Dauerübersteuerung der Rest der Programmschleife dank Instantreaktion im Dzug Tempo abläuft, statt im Sekundenrhythmus.

				BTFSC  vMyFlags,6
				GOTO   Messen

 ;LichtMeldung---------------------------------------------

			CALL  	Meldeton  ;  Einmal pro Sekunde Das MeldetonModul rufen, um den Meldeintervallzähler weiterzusetzen
								;	oder ggf einen Meldeton zu senden.
 ;Betriebsstunden-Limitierung ---------------------------------------------------------

			CALL  Limit

   ; Ausschaltung/Entscheidung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------
   ; Ausschaltung/Entscheidung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------
   ; Ausschaltung/Entscheidung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------Ausschaltung: XXXXXXXXXXXXXX-----------

Licht1				DECFSZ  vBeschleuniger,F
					GOTO  AbsoLaut
					MOVLW  LichtBewNenn             ;Hier lasssen sich verschiedene Verhältnisse einstellen um das gewünschte timing einzustellen Standard=3!!!!!!!!!!!!!!!!!
					MOVWF  vBeschleuniger			; also alle wieviel Sekunden die Licht und Kompensationsbonusvergabe durchlaufen wird und wieviele dann jeweils vergeben.
Licht2				BTFSC PORTA,4					; LichtBonus    Pin3 /RA4 =Low  d.h. es ist hell am Lichtsensor
					GOTO  Licht3
					MOVLW  LichtanBonus        ;Was nur passiert, wenn es hell ist:
					ADDWF  vAusschaltBonus,F
					MOVLW  LichtanMalus
					SUBWF  vAusschaltBonus,F
				          
Licht3				MOVLW  LichtKompensBonus  ;Was jedesmal passiert, wenn die Helligkeit ausgewertet wird:
					ADDWF  vAusschaltBonus,F
					MOVLW  LichtKompensMalus  ;Standard=2 für Zimmer u.ä. schaltet es sonst einfach zu bummelig ab. Schon ohne Licht.Also jede 3. Sekunde noch zwei runter.
					SUBWF  vAusschaltBonus,F

					DECFSZ vFrustBerücksi,F
					GOTO  AbsoLaut
					MOVLW  FrustBerückNenn
					MOVWF  vFrustBerücksi
					CALL  FrustBonus	; hier fließt die Frustauswertung in die Ausschaltung ein  (sehr langfristig)bis zu zwei extra decf s werden vergeben.


AbsoLaut       	DECFSZ  vAbsoZaehl,F   ; in diesem Absatz kann man einstellen, 
			 	GOTO  Klassen11          ;wieviel die absolute Lautstärke zur Erhaltung des Einschaltzustandes beitragen soll:
				MOVLW  AbsolautNenn     ;wie oft
				MOVWF  vAbsoZaehl
				MovlW  AbsolautLo     ;für welche Pegel
				Subwf  vKlassenSumme,W
				BTFSS  STATUS,C
				Goto   Klassen11

				MOVLW  AbsoLoBonus
				ADDWF  vAusschaltBonus,F   ;wieviele Bonusse vergeben werden.

				MovlW  AbsolautHi  ;für welche Pegel
				Subwf  vKlassenSumme,W
				BTFSS  STATUS,C
				Goto   Klassen11
				MOVLW  AbsoHiBonus
				ADDWF  vAusschaltBonus,F   ;wieviele Bonusse vergeben werden.				


Klassen11	BCF   vMyFlags,5
	DecFSZ vKlassenZahl,F          ;Auswertung der Klassenzahl also der Lautstärkevarianz innerhalb einer Sekunde
	GOTO  Klassen2
		MOVLW EinklassenMalus
		SUBWF vAusschaltBonus,F        ;wenn nur eine Lautstärkeklasse war, dann war das typisch für Störgeräusche wie Presslufthämmer etc.Oder ganz leise. 


Klassen7	BSF vMyFlags,1
	Goto KlassenFertig

Klassen2 DECFSZ vKlassenZahl,F     ; zwei Klassen vertreten: Unentschieden keine Aktion
		GOTO Klassen3
		BSF vMyFlags,1
		GOTO  KlassenFertig
		
Klassen3    BCF vMyFlags,1
			BSF  vMyFlags,5
			DECFSZ vKlassenZahl,F
			GOTO  Klassen4
			MOVLW DreiklassenBonus   ;Drei Lautstärkeklassen vertreten: Könnte Sprache sein.  Bonus
			ADDWF vAusschaltBonus,F

			GOTO  KlassenFertig

Klassen4   MOVLW VierklassenBonus
			ADDWF   vAusschaltBonus,F   ; vier  Lautstärkeklassen:  Dreifach bonusse (Standard)


KlassenFertig  	MOVLW OfenausZahl                 ;Entscheidung ob ausgeschaltet wird fällt hier.
				SUBWF   vAusschaltBonus,W
				BTFSS  STATUS,C
				GOTO  Abschluss
	
				MOVLW OfenheissZahl					;Überlaufschutz Ausschaltbonus  Unterlauf erübrigt sich da dann Ausschaltung
				ADDLW   D'1'
				SUBWF   vAusschaltBonus,W
				BTFSS	STATUS,C
				GOTO neueRunde
				MOVLW OfenheissZahl  
				MOVWF vAusschaltBonus
neueRunde		CALL EreignisZaehler
				GOTO Messen
			

Abschluss   BSF  vMyFlags,0
				CALL EreignisZaehler
				RETFIE				;Geht an die Stelle nach dem Interrupt-Aufwachen also zum Befehl nach sleep (und von da direkt wieder schlafen.)

                              ;!    !  ! Das Ausschalten !

;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
			;Frust-Steuerung: Wenn die Maschine selbst ein Frustrationsverhalten zeigt, werden dem Zuhörer Frustrationen erspart:
EreignisZaehler  	BTFSS   vMyFlags,5
					GOTO   Zaehler60
					INCF   vEreignis,F     ;Wenn 3 oder 4 Lautstärkeklassen im letzten Durchgang waren
Zaehler60				DECFSZ  vZaehl60,F
						Goto AusschaltFrage
						CALL   EreignisAuswert      ;alle 60 Sekunden, oder...      
						return
AusschaltFrage		 	BTFSS   vMyFlags,0			;... wenn ausgeschaltet werden soll.
						return
						CALL   EreignisAuswert							
						return       


 ;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
EreignisAuswert  	MOVLW   D'1'								;hier gehen Calls ein !
						SUBWF		 vEreignis,W				 ;Es hat sich erwiesen, dass solche 3/4 Klassen - Ereignisse seltener sind, als gedacht.
						BTFSC  STATUS,C							; deshalb Grenzen reduziert.
						Goto    Ereignis4
						decf    vFrust,F						;Je nachdem wie oft für diese Einschaltung bzw. in den letzten 60Sekunden die 
						decf    vFrust,F						; dreiodervierKlassen-Ereignisse waren wird vFrust runter oder rauf gezählt.
						GOTO    EreignisClr
Ereignis4			MOVLW   D'3'
						SUBWF		 vEreignis,W				 
						BTFSC  STATUS,C
						Goto    Ereignis10
						decf    vFrust,F
						GOTO    EreignisClr
Ereignis10			MOVLW   D'7'
						SUBWF		 vEreignis,W				 
						BTFSS  STATUS,C
						GOTO    EreignisClr
						incf    vFrust,F						
					MOVLW   D'14'
						SUBWF		 vEreignis,W				 
						BTFSS  STATUS,C
						GOTO    EreignisClr
						incf    vFrust,F

EreignisClr				CLRF  vEreignis
						MOVLW  D'60'
						MOVWF		vZaehl60


FrustProbe				MOVLW   D'40'           ; und wenn vFrust dann 40 unter oder 80überschreitet, dann hat das Folgen.
						SUBWF		 vFrust,W				 ;(Die VoxEmpfindlichkeit und danach auch Ausschaltgeduld wird verstellt.)
						BTFSC  STATUS,C
						Goto    Frust80
						DECF		vVoxIstStufe,F
						MOVLW  D'60'
						MOVWF		vFrust
						GOTO		VoxStell
Frust80				MOVLW   D'80'
						SUBWF		 vFrust,W				 
						BTFSS  STATUS,C
						Goto    VoxStell
						INCF		vVoxIstStufe,F
						MOVLW  D'60'
						MOVWF		vFrust
						

			org D'420'
VoxStell	BSF STATUS,RP0                 ; Es gibt drei Voxstufen Entspr 59, 60 ,61  hier werden die Ausgänge entsprechend eingestellt und 
			MOVLW D'1'						;  das zugehörige Register begrenzt:    61= empfindlich 59=une.
			MOVWF PCLATH
			MOVLW D'60'
			SUBWF 	vVoxIstStufe,W
			BTFSC   STATUS,C
			GOTO   Vox60
			MOVLW D'0'
			CALL  VoxStu  ;dieser Programmteil stellt dann die Überschneidung mit den Voreinstellungen für die Abhörumgebung her.und stellt die Ausgänge ein.

			MOVLW D'59'
			MOVWF vVoxIstStufe
			GOTO	VoxStellEnde
Vox60		MOVLW D'60'
			SUBWF 	vVoxIstStufe,W
			BTFSS   STATUS,Z
			GOTO  Vox61
			MOVLW D'1'
			CALL  VoxStu  ;dieser Programmteil stellt dann die Überschneidung mit den Voreinstellungen für die Abhörumgebung her.und stellt die Ausgänge ein.
			GOTO	VoxStellEnde
Vox61		MOVLW D'2'          ;hier angekommen, dh größer 60
			CALL  VoxStu ;dieser Programmteil stellt dann die Überschneidung mit den Voreinstellungen für die Abhörumgebung her.und stellt die Ausgänge ein.
		
			MOVLW D'61'
			MOVWF vVoxIstStufe

VoxStellEnde		BCF STATUS,RP0
					
  			 return       

									;ENDE DER HAUPTSCHLEIFE!
									;ENDE DER HAUPTSCHLEIFE!
									;ENDE DER HAUPTSCHLEIFE!


  ;Hier wird die Vox variabel eingestellt. Je nach dem oben im EQU-Teil angegebenen Stufe 1bis 5 werden die empfindlichsten oder unempfindlichsten Vox-Stufen deaktiviert.
	      ;damit das nicht grade auf einer Memory-Bereichsgrenze liegt. Genauer Wert müßte bestimmt werden wenn Memory knapp wäre.
						;denn natürlich ist hier wesentlich weniger als der 768 ste Befehl.(ca500?)Aber garantiert nicht mehr.
						;Ziemlich viel Aufwand, nur um die Einstellung in den Kopf-Teil zu verlagern.
VoxStu  ADDWF PCL,F	
		GOTO VoxStulau    ;Die Vox will sich auf unempfindlich schalten,weil es laut ist. je nach voxoffset wird ihr das auch erlaubt.
		GOTO VoxStuMitt   ;die Vox will mittel...s.o.
		GOTO VoxStuleis		;s.o.

VoxStulau   MOVLW Voxoffset
			ADDWF PCL,F
		GOTO  Voxunempf
		GOTO  Voxunempf
		GOTO  Voxunempf
		GOTO  Voxmittel
		GOTO  Voxempfind

VoxStuMitt   MOVLW Voxoffset
			ADDWF PCL,F
		GOTO  Voxunempf
		GOTO  Voxmittel
		GOTO  Voxmittel
		GOTO  Voxmittel
		GOTO  Voxempfind

VoxStuleis   MOVLW Voxoffset
			ADDWF PCL,F
		GOTO  Voxunempf
		GOTO  Voxmittel
		GOTO  Voxempfind
		GOTO  Voxempfind
		GOTO  Voxempfind
			

Voxunempf			BSF TRISA,6
					BSF TRISA,7
				return

Voxmittel  			BSF TRISA,6
					BCF TRISA,7
				return

Voxempfind  		BCF TRISA,6
					BSF TRISA,7
				return












LilaLaune     EQU    D'100'     



;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----

;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----

;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----

;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
;SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----SPANNGSWANDLER----
  ;SPANNGSWANDLER Einschalten -----------------XXXXXXXXXXX --UND ANDERE VOREINSTELLUNGEN----- ;SPANNGSWANDLER Einschalten -----------------XXXXXXXXXXX --UND ANDERE VOREINSTELLUNGEN-----
          ;UND ANDERE VOREINSTELLUNGEN----- ;SPANNGSWANDLER Einschalten ----------
WInit NOP
TmrInit Movlw B'00100000'        ;Timer 0 in Gang setzen für regelmäßige Nachregelung der PWM (im 100 Hertz Takt oder so)
		MOVWF INTCON

   BSF STATUS,RP0
	MOVLW B'10001010'         ;Timer 0 prescaler, keine Pull up's, neg Flanke für RB0-Interrupt 
								; Bit 3 gesetted = Kein Prescaler =8xschneller als ursprüngl  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	MOVWF OPTION_REG			;mit diesen Einstellungen bloß keinen Mist machen, weil der Schalttransistor sonst ev Kurzschluß macht 
								;und Rauch und kaputt. Wahrsch Spule. dann keine 10 V mehr für NF und Sender !!!!!!!!!!!!!!!!!!!!!!!!!!		
	BCF STATUS,RP0

		BCF PORTA,6  ;Vox-Vu RegelPins sicherstellen, dass beide gleiches Potential vermeidet Strom
		BCF PORTA,7


PWMInit NOP
	BSF STATUS,RP0
	MOVLW PWMPeriode               
	MOVWF PR2
	BCF TRISB,3

	BCF STATUS,RP0
   	MOVLW B'00001100'
	MOVWF CCP1CON
	MOVLW B'00000100'
	MOVWF CCPR1L
	BSF T2CON,2


RegelInit	MOVLW B'00000111'      ;Bloß keinen Mist machen! muß immer ein Drittel größer als CCPR1L.Manual nachlesen! !!!!!!!!!!!!!!!!!!!!!!!!!
	MOVWF CMCON
	BSF STATUS,RP0
	BCF TRISA,3     ;(Bei Gelegenheit auch gleich den Ausgang für die Sendestufen-Einschaltung konfigurieren)
	BSF TRISA,0		;PWM Regeleingang
	BCF STATUS,RP0

	RETFIE


      
 ; INTERRUPT ROUTINE FÜR SPANNUNGSWANDLER REGELUNG  XXXXX------XXXXX------XXXXX------XXXXX------
 ;-------PWM-RegelungXXXXX---------PWM-RegelungXXXXX---------PWM-RegelungXXXXX---------PWM-RegelungXXXXX---------PWM-RegelungXXXXX------


PWMStart    movwf   WTemp                 ; Status und Arbeitsspeicher sichern
    		swapf   STATUS,W 
    		bcf     STATUS, RP0       ; status_temp in Bank 0 
    		movwf   StatusTemp 



	BCF PORTB,3         ;gegen inout-Falle durch Tt

		
Regel MOVF CCPR1L,W
	MOVWF vPWMCon
	RLF vPWMCon,F
	RLF vPWMCon,F

kopieren6	BTFSC CCP1CON,4   
		GOTO kopieren7
		BCF vPWMCon,0      
		GOTO kopieren8
kopieren7 BSF vPWMCon,0

kopieren8	BTFSC CCP1CON,5   
		GOTO kopieren9
		BCF vPWMCon,1      
		GOTO kopieren10
kopieren9 BSF vPWMCon,1


kopieren10	 BTFSC PORTA,0  ;Spanngswandler auf Überspannung prüfen
		GOTO RegelDown

RegelUp	INCF vPWMCon,F   ;Power hochschalten
	MOVF vPWMCon,W
	SUBLW  B'00001000'         ;Hier PWM max Impulsdauer einstellen ACHTUNG: Max 10000. !!!!!!!!!!!!!!!!!!!!!!!!!!!
	BTFSC STATUS,C				;10000 entspricht eigentlich 100,00 in CCPR1L    Bloß keinen Mist machen hier! Hardware in Gefahr !!!!!!!!!!
	GOTO kopieren1
	DECF vPWMCon,F
	GOTO kopieren1

  	  ;Power runterschalten
RegelDown	MOVLW 0x01
	SUBWF vPWMCon,F
	BTFSC STATUS,C
	GOTO kopieren1
	INCF vPWMCon,F
	GOTO kopieren1


  
kopieren1	BTFSC vPWMCon,0   ;Bits in die PWM-Steuer-Register kopieren
		GOTO kopieren2
		BCF CCP1CON,4      ;Least sign. Bit von CCPR1L
		GOTO kopieren3
kopieren2 BSF CCP1CON,4

kopieren3	BTFSC vPWMCon,1   ;2.Bit in die PWM-Steuer-Register kopieren
		GOTO kopieren4
		BCF CCP1CON,5      ;Least sign. Bit von CCPR1L
		GOTO kopieren5
kopieren4 BSF CCP1CON,5

kopieren5 MOVF vPWMCon,W
		MOVWF vCopyRota
		RRF vCopyRota,F
		RRF vCopyRota,F
		BCF vCopyRota,7
		BCF vCopyRota,6
		MOVF vCopyRota,W
		MOVWF CCPR1L


 ;------------------------------------------------
	
	 BCF INTCON,2     ;Interrupt reaktivieren keine Eile, da doch grade erst gestartet.
    swapf   StatusTemp,W    ;Status und Arbeitsspeicher wiederherstellen
    movwf   STATUS 
    swapf   WTemp,F 
    swapf   WTemp,W 

    retfie 


;XXXXXX XXXXXX XXXXxxxx xxxxxxx XXXXXX XXXXX XXXxxxxxxxxxx xxxxxxxX  XXXXXXXX XXXXX
 ;--------ENDE PWM RegelungXXXXX--------ENDE PWM RegelungXXXXX;--------ENDE PWM RegelungXXXXX--------ENDE PWM RegelungXXXXX
 ;--ENDE PWM RegelungXXXXX--------ENDE PWM RegelungXXXXX;--------ENDE PWM RegelungXXXXX--------ENDE PWM RegelungXXXXX--------






     ;ZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääää
;ZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääääZeitschleifenäääääääääääää

;Schl20Reg  EQU   0xxxxx
Schl20     Movlw D'5'    ;20-Sekunden-Schleife
	Movwf Schl20Reg
Schl201	Call Schl1M
    DECFSZ Schl20Reg,F
  	Goto Schl201   
	return	


	


;Kor1MReg   	EQU     0xxxx		;plus Konstante-Register
;Inn1MReg   	EQU     0xxxx		;innere Schleifenzahl mal
;Auss1MReg	EQU     0xxxx		;aeussere Schleifenzahl-Reg
;SupAuss1M   EQU		0xxxx		;ganz äussere SchlZahl-Register
Schl1M		MOVLW   0x7F		;Korrekturschleife laden
			MOVWF   Kor1MReg
Schl1MKorr	DECFSZ  Kor1MReg,1
			GOTO    Schl1MKorr	
			MOVLW   0x8C		;Superaeussere Schl laden
			MOVWF   SupAuss1M	
load1Mauss	MOVLW 	0x2B		;Schl auss laden
 			MOVWF 	Auss1MReg 
load1Minn	MOVLW	0x51		;Schl  inn laden
 			MOVWF 	Inn1MReg
Schl1Minn	DECFSZ	Inn1MReg,1
			GOTO    Schl1Minn
            NOP
			DECFSZ 	Auss1MReg,1
			GOTO    load1Minn
			DECFSZ  SupAuss1M,F
			GOTO    load1Mauss
			Return


zehnMue NOP
		NOP
		NOP
		NOP
		NOP
		NOP
	RETURN


Schl200    	CALL Schl100
			CALL Schl100
			RETURN


 ;Kor100Reg   EQU     0x1B		;plus Konstante
 ;Inn100Reg   EQU     0x1A		;innere Schleifenzahl mal
 ;Auss100Reg	EQU     0x19		;aeussere Schleifenzahl
Schl100		MOVLW   0x2F		;Korrekturschleife laden
			MOVWF   Kor100Reg
Schl100Korr	DECFSZ  Kor100Reg,1                              ;Korrektur/ Schleifenlänge um Mess und Auswertezeit verkürzen
			GOTO    Schl100Korr		
			MOVLW 	0xDE		;Schl auss laden
 			MOVWF 	Auss100Reg 
load100inn	MOVLW	0xDF		;Schl  inn laden
 			MOVWF 	Inn100Reg
Schl100inn	DECFSZ	Inn100Reg,1
			GOTO    Schl100inn
            CLRWDT
			DECFSZ 	Auss100Reg,1
			GOTO    load100inn
			Return

;-------------------------------------------------------------------
;----------------------------------------------------------------
				org D'790'
Startverzögerung CALL Schl1M


							

				BSF STATUS,RP0
				BCF  PCON,3      ;auf stromsparende 37khz umschalten
				BCF STATUS,RP0

				MOVLW Einschleichzeit      ;Startverzögerungszeit in Stunden/Minuten  0-254
				MOVWF vStart1
				INCF vStart1,F
StartVerz1		DECFSZ		vStart1,F
				GOTO StartVerz2
				GOTO WiederVier
StartVerz2      Call MiStuSchleife
				GOTO  StartVerz1
				

MiStuSchleife   MOVLW D'3'
				MOVWF PCLATH
				MOVLW 	Einschleichmassstab					
				ADDWF  PCL,F                 
				RETURN
				GOTO  Minutenschleife
				Goto  Stundenschleife
				RETURN				;nur zur Sicherheit, falls doch mal versucht wird, 3oder 4 einzutragen.Eigentlich kommt man hier nie vorbei.
				Return					; so merkt man's gleich.

				
Stundenschleife MOVLW D'5'         ; die beiden Stu/Mi Schleifen sind nicht ganz exact auf den richtigen Wert eingestellt, da die 
				MOVWF vXh3			;Quarzlosen internen Oszillatoren sowieso nur ungefähr ein Prozent genau sind. 
				CLRF  vSupAuss		; Mithilfe einer Stoppuhr hab ich die 10 Minuten und 1Stunde auf etwa eine Sekunde genau bei Zimmertemperatur eingestellt
				CLRF  vAuss			; Man sollte aber lieber mit 15 Sek Abweichung pro Stunde rechnen 1Prozent wäre 36.
				CLRF  vInn			;Im Einschleichbetrieb verbraucht die Schaltung ca 250 bis 300 uA bei UB=3V - 3,6V  Darüber noch etwas mehr.Aber unter 1mA
Stuinn			MOVLW  D'40'		;Im Sleep Betrieb mit Vox wake - up nur etwa 110uA  (90uA bei 2,8V)
				MOVWF  vInn
Stuinn0		    DECFSZ vInn,F
				GOTO  Stuinn0
			DecFSZ vAuss,F
				GOTO  Stuinn
          DECFSZ vSupAuss,F
				GOTO  Stuinn
				DECFSZ vXh3,F
				GOTO  Stuinn

Stuinn3      	MOVLW D'102'
				MOVWF  vInn

Stuinn1			DECFSZ vInn,F
				GOTO  Stuinn1
		DecFSZ vAuss,F
				GOTO  Stuinn3
          DECFSZ vSupAuss,F
				GOTO  Stuinn3

				return


Minutenschleife  MOVLW D'1'
				MOVWF vXh3
				CLRF  vSupAuss
				CLRF  vAuss
				CLRF  vInn
Stuinn4			MOVLW  D'1'
				MOVWF  vInn
Stuinn5		    DECFSZ vInn,F
				GOTO  Stuinn5
				DecFSZ vAuss,F
				GOTO  Stuinn4
        		DECFSZ vSupAuss,F
				GOTO  Stuinn4
				DECFSZ vXh3,F
				GOTO  Stuinn4

				MOVLW D'81'
				MOVWF  vSupAuss
Stuinn8      	MOVLW D'7'
				MOVWF  vInn


Stuinn9			DECFSZ vInn,F
				GOTO  Stuinn9
				DecFSZ vAuss,F
				GOTO  Stuinn8
          		DECFSZ vSupAuss,F
				GOTO  Stuinn8

				return




WiederVier		BSF STATUS,RP0
				BSF  PCON,3      ;von 37kHz wieder auf die für das Hauptprogramm verwendeten 4MHZ zurückschalten.
				BCF STATUS,RP0
				RETURN                     ;Ende des Startverzögerungsprogrammes---------

  ;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

 ;MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

Meldeton  	CLRW
			ADDLW     MeldeIntervall
			BTFSC    STATUS,Z
			RETURN

		 	DECFSZ vMeldeIntervall,F
			RETURN

			BCF  vMyFlagsB,1  ;MultibankAdresse! My-Flag's Register2
			BSF  STATUS,RP0
			BTFSC  TRISB,7
			BSF  vMyFlagsB,1   ;Das Flag, dass der Toneinspieleingang aus Gründen der Verstärkungsregelung gesettet war, bzw. nicht.
			BCF  TRISB,7			;(für das Einspielen des Tons.)
			BCF  STATUS,RP0

			MOVLW   MeldeIntervall
			MOVWF	vMeldeIntervall
			BTFSC   PORTA,4				;LichtPort prüfen
			CALL    EinTon      ;dunkel = zwei Töne
			CALL    kEinTon       ;hell = ein Ton 
			CALL    EinTon

			BTFSS  vMyFlagsB,1  ;Gesicherten Zustand des TRISB Ausgangs wiederherstellen.
			RETURN
			BSF  STATUS,RP0
			BSF  TRISB,7			;(für das Einspielen des Tons.)
			BCF  STATUS,RP0
			CALL    kEinTon
			RETURN

			
EinTon		BCF  INTCON,7
			Movlw D'254'      ;ACHTUNG!!! Muß unbedingt eine grade Zahl sein, damit der Sendepower Ausgang hinterher denselben Wert hat, wie zuvor.
			Movwf  vTonADauer		

EinTon2		MOVLW D'60'
			MOVWF  vTonAFreq   ;  Vonwegen A ! Bei dem Grad an Übersteuerung latcht sich der OP fest und kommt mit Ton A garnicht mit.
EinTon1			NOP


			DECFSZ	vTonAFreq,F
			GOTO	EinTon1
			MOVLW  B'10000000'  ;PortB,7 umschalten
			XORWF  PORTB,F
			DECFSZ  vTonADauer,F
			GOTO	EinTon2
			BSF  INTCON,7
			RETURN

kEinTon		Movlw D'176'
			Movwf  vTonADauer

kEinTon2		MOVLW D'188'
			MOVWF  vTonAFreq
kEinTon1			NOP
			DECFSZ	vTonAFreq,F
			GOTO	kEinTon1
			MOVLW  B'00000000'  ;PortB,7 --- nicht --- umschalten
			XORWF  PORTB,F
			DECFSZ  vTonADauer,F
			GOTO	kEinTon2
			RETURN
  ;LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
Limit   DECFSZ vLimitCounterA,F
		Return
		CALL  Pruefen
		DECFSZ vLimitCounterB,F
		Return
		MOVLW  D'7'       			;hier kommt man nur noch ungefähr einmal pro Sendestunde hin. (+/- 30Prozent also sehr ungenau)
		MOVWF  vLimitCounterB
		CALL  EEPROMEintrag
		Return
				        
		 
Pruefen		CALL Lesen
			SUBLW LilaLaune
			BTFSC STATUS,C
			RETURN

Feierabend   BSF   STATUS,RP0      ;PWM aus
	BCF  VRCON,7      ;alle Stromverbraucher abschalten
	CLRF TRISB
	BSF  TRISB,0
	BCF STATUS,RP0

	MOVLW   B'00000111'
	MOVWF   CMCON
    BCF INTCON,5         ;timer0 disablen (PWM Stellzyklus-Timer)
	CLRF CCP1CON       ;PWM aus mit output low
	BCF T2CON,2
	BCF PORTA,3    ;Sendestufe ausschalten
	BCF PORTB,3			;da P6 jetzt normaler inout-Pin den lieber auch nochmal low, sonst ist Der PWM-SchaltFet im Kurzschluss
	BCF INTCON,4		;Disable VOX Interrupt
	BCF INTCON,7		; GIE
	
  				BSF STATUS,RP0
				BCF  PCON,3      ;auf stromsparende 37khz umschalten
				BCF STATUS,RP0	
Wait        CALL  Minutenschleife
			DECFSZ vFeierSignal,F  
			GOTO  Wait
			MOVLW D'15'
			MOVWF  vFeierSignal
				


				BSF STATUS,RP0
				BSF  PCON,3      ;von stromsparenden 37khz zurückschalten
				BCF STATUS,RP0	

		CALL  WInit
		CALL  Schl1M
	   BSF PORTA,3        ;Sendestufe einschalten
		CALL  Schl1M
	   BCF PORTA,3    ;Sendestufe ausschalten
		CALL Schl1M
	   BSF PORTA,3        ;Sendestufe einschalten
		CALL  Schl1M
	   BCF PORTA,3        ;Sendestufe ausschalten
	
		GOTO Feierabend
;-------------------------------------------------

Lesen		BSF STATUS,RP0  ;EEPROM lesen
			MOVLW eLoEEP
			MOVWF  EEADR
			BSF  EECON1,0
			MOVF EEDATA,W
			BCF STATUS,RP0
			RETURN
;-----------------------------------------------------------------


;-------------------------------------------------------------------------------------------------------------------------------
							  

		; BetriebszeitZähler              Incrementiert das EEprom Betriebszeitregister bei jedem Aufruf  also ca jede Stunde
		
		;BetriebszeitZählerEEPROMS updaten
;vLoBysav    	EQU   0x22		;RAM Adr	BANK1	
;vUpBysav   	EQU   0x23 		;RAM Adr    BANK1
;eUpEEP		EQU	  0x03		;EEPROM Adr
;eLoEEP    	EQU   0x04		;EEPROM Adr

		;EEprom auslesen low Byte
  
EEPROMEintrag	BSF  STATUS,RP0
 				MOVLW  eUpEEP 			;EEprom auslesen Up Byte
		 		MOVWF  EEADR
				BSF    EECON1,RD
				MOVF   EEDATA,W
				MOVWF  vUpBysav     ;zu späterer Verwendung

            MOVLW  eLoEEP 		;Low Byte lesen	
		 	MOVWF  EEADR
			BSF    EECON1,RD
			MOVF   EEDATA,W
			MOVWF  vLoBysav
			MOVLW  D'1'
			ADDWF  vLoBysav,F
			BTFSC  STATUS,C
            CALL   upByte       ; Bedingter Aufruf des Upbyte-Updatings
									   ;und weiter...
		;geupdatetes LoByte ins EEPROM
			MOVLW	eLoEEP			;Adresse und Daten laden low Byte
			MOVWF   EEADR
			MOVF	vLoBysav,W
			MOVWF	EEDATA


	BCF    INTCON,GIE      ;Interrupts sperren			;Schreibroutine Start
			BSF    EECON1,WREN
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR
PollEE5		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE5
	BsF    INTCON,GIE      ;Interrupts erlauben


			BCF    EECON1,WREN
			BCF    STATUS,RP0
			RETURN		
		;SchreibRoutine Ende

upByte			MOVLW  D'1'
				ADDWF  vUpBysav,F

			MOVLW	eUpEEP			;EEprom Schreiben- Adresse und Daten laden Up Byte
			MOVWF   EEADR
			MOVF	vUpBysav,W
			MOVWF	EEDATA

			BSF    EECON1,WREN									
				BCF    INTCON,GIE      ;Interrupts sperren		;Schreibroutine Start
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR
PollEE6		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE6
				BSF    INTCON,GIE      ;Interrupts sperren		;SchreibRoutine END
			RETURN

			;Ende des BetrZähl - ZeitSchleifen als Schleifen einsortiert


;PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP

 ;PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
  
PrimInit 
		 MOVLW  D'62'                ;Diese Einstellungen werden nur beim ersten Einschalten nach dem Batterieanschluß geladen.
		MOVWF  vVoreinstellVU
		BCF  PORTA,3
		MOVLW  PrimstartRegelZahl
		MOVWF   vRegelZahl
		MOVLW  PrimstartRegelZahl
		ADDLW  D'253'         ;Komplementaddition = minus 3
		MOVWF  vRegelZahlSum

		CALL  InitBetr  ;Überprüfung des EEProm



			BSF STATUS,RP0
			MOVLW  B'01111111'
			MOVWF  TRISB 
			MOVLW  B'01110111'
			MOVWF  TRISA			
			BCF STATUS,RP0
			CLRF  PORTB 
			MOVLW B'00000010'
			MOVWF  vMyFlags
			MOVLW  D'61'   
			MOVWF vVoxIstStufe
						;MOVLW  D'60'
			MOVWF		vFrust
					MOVLW  D'2'                 
					MOVWF  vBeschleuniger

				MOVLW  D'35'
				MOVWF vLimitCounterA
				MOVLW  D'8'
				MOVWF vLimitCounterB
		MOVLW D'5'
		MOVWF vFeierSignal



				CLRF vPegel1      
				CLRF vPegel2
				CLRF vPegel3
				CLRF vPegel4
				CLRF vPegel5
		
		RETURN

;--------------------------------------------------------------------------------------------------------------
;-----------------------------------------------------------------------------------------------------------

;URINBETRIEBNAHME  NACH DEM PROGRAMMIEREN EINMALIG

;FÜR BETRIEBSSTUNDENZÄHLER
;Cleart das Betriebsstunden EEprom, was danach natürlich keinesfalls mehr vorkommen darf.
;wird nur beim Anlegen der Betriebsspannung überprüft.

   							;von Priminit gecallt


		;eInitproof1	EQU    0x01   ;EEprom Adresse
		;eInitproof2  EQU    0x02		;EEPROM Adr
		;vProoftemp1	EQU   jhlz 0x24	hz	;RAM Adresse   BANK1 Adressen
		;vProoftemp2	EQU   ftz 0x25	gh	;RAM Adr
InitBetr  	BSF    STATUS,RP0
 			BCF    INTCON,GIE   ;EEprom auslesen  Löschbestätigung1
            MOVLW  eInitproof1 			 
		 	MOVWF  EEADR

			BSF    EECON1,RD
			MOVF   EEDATA,W
			MOVWF  vProoftemp1
			
            MOVLW  eInitproof2 	;EEPROM auslesen	Löschbestätigung2 
		 	MOVWF  EEADR
			BSF    EECON1,RD
			MOVF   EEDATA,W
			MOVWF  vProoftemp2

			MOVLW  0x5B				;Lösch 1 prüfen  (nur das letzte Bit mußund darf vershieden sein, und das wird dann decrementiert.)
			XORWF  vProoftemp1,F    
			DECFSZ vProoftemp1,F
			GOTO   Goon1
			BCF    STATUS,RP0
            RETURN
            	
Goon1		MOVLW  0x5A				;Lösch 2 prüfen
			XORWF  vProoftemp2,F    
			DECFSZ vProoftemp2,F
			GOTO   Goon2
			BCF    STATUS,RP0
            RETURN

 			;InitialSetzung EEprom BetrZähler  auf 1
Goon2		MOVLW	eLoEEP			;Adresse und Daten laden low Byte
			MOVWF   EEADR
			MOVLW	D'1'
			MOVWF	EEDATA
										;Schreibroutine Start
			BSF    EECON1,WREN
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR	
PollEE1		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE1
										
   			MOVLW	eUpEEP			;Adresse und Daten laden UP Byte
			MOVWF   EEADR
			MOVLW	D'0'
			MOVWF	EEDATA
										;Schreibroutine Start
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR						
PollEE2		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE2				

			;InitBestätigung1 schreiben
   			MOVLW	eInitproof1			;Adresse und Daten laden 
			MOVWF   EEADR
			MOVLW	0x5A
			MOVWF	EEDATA
								;Schreibroutine Start
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR		
PollEE3		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE3

  			MOVLW	eInitproof2			;Adresse und Daten laden 
			MOVWF   EEADR
			MOVLW	0x5B
			MOVWF	EEDATA
								;Schreibroutine Start
			MOVLW  0x55
			MOVWF  EECON2
			MOVLW  0xAA
			MOVWF  EECON2
			BSF    EECON1,WR	
PollEE4		BTFSC  EECON1,WR      ;Schreibabwarten
			GOTO   PollEE4

			BCF    EECON1,WREN
			BCF   STATUS,RP0
			RETURN
	    ; ENDE DER INITIALISIERUNG desBetrZählers

     ;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     ;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


	 ; Einschub Frust
FrustBonus	MOVLW 		D'61'		; gecalled. hier fließt die Frustauswertung in die Ausschaltung ein  (sehr langfristig)
			SUBWF 		vVoxIstStufe,W				
			BTFSC  		STATUS,C													
			RETURN
			MOVLW    FrustBerueckZaehler
			SUBWF  	vAusschaltBonus,F

 	
			MOVLW 		D'60'
			SUBWF 		vVoxIstStufe,W
			BTFSC		STATUS,C
			RETURN
			MOVLW    FrustBerueckZaehler
			SUBWF  	vAusschaltBonus,F
			return
	; Ende Einschub Frust





     ;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

				;Auswerten des Übersteuerugsspeichers - Stellen der Regelzahlen
UeAuswert		MOVF vPegel5,W
				ADDWF vUebersteuerung,F
				MOVLW  UeGeduldEnde        
				SUBWF vUebersteuerung,W
				BTFSS  STATUS,C
				GOTO   Untergrenze
				DECF  vRegelZahl,F
				DECF  vRegelZahlSum,F
				MOVLW  D'60'				;(zurücksetzen wichtig, damit eine Änderung der Regelzahlen nicht zu schnell weitere nach sich zieht.)
				MOVWF  vUebersteuerung						;ausserdem damit automatisch Überlaufschutz.

Untergrenze		MOVLW  MehrUeWagen   
				SUBWF vUebersteuerung,W
				BTFSC  STATUS,C
				GOTO   RegelZUeber
				INCF  vRegelZahl,F
				INCF  vRegelZahlSum,F
				MOVLW  D'60'				;dto
				MOVWF  vUebersteuerung
				
RegelZUeber		MOVLW  RegelZOG
				ADDLW  D'1'
				SUBWF  vRegelZahl,W
				BTFSS	STATUS,C
				GOTO	RegelZUnt
				MOVLW  RegelZOG			;(Die beiden Zahlen haben immer konstanten Abstand)
				MOVWF  vRegelZahl
				MOVLW  RegelZOG
				ADDLW   D'253'     ;Komplementaddition, also minus3
				MOVWF  vRegelZahlSum

RegelZUnt		MOVLW  RegelZUG
				SUBWF  vRegelZahl,W
				BTFSC	STATUS,C
				GOTO	Relevanz9
				MOVLW  RegelZUG
				MOVWF  vRegelZahl
				MOVLW  RegelZUG
				ADDLW   D'253'     ;Komplementaddition, also minus3
				MOVWF  vRegelZahlSum
Relevanz9				Return


;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

	      
	

  
VUAusgabe		MOVF vVUistStufe,W
			BSF STATUS,RP0
			MOVWF 	vVUistBank1    ;Hat auch Datenpufferwirkung für untiges Auswertverfahren 
									;das die ursprünglichen Daten verdirbt nicht nur wegen Bänken


			MOVLW D'60'						
			SUBWF  vVUistBank1,F		; WerteBereich 60-66
			BTFSS  STATUS,Z
			GOTO   AusgTest2
			BsF TRISB,7			;größter R  also VU zweitniedrigste Stufe nach gar-kein-Port-gecleared
			BsF TRISB,6						;(jedesmal die Speicher einzeln stellen umständlich, aber vermeidet knacks effekt)
			BsF TRISB,5
			BsF TRISB,4
								;Trisb, 3 steht für vu nicht zur Verfügung  (PWM-Ausgsang für Spannungswandler)
			BsF TRISB,2
			BsF TRISB,1      ;kleinster R also höchste VU
			GOTO	VUAusFertig		

AusgTest2	DECF   vVUistBank1,F
			BTFSS  STATUS,Z
			GOTO   AusgTest3
			BCF TRISB,7	

			BsF TRISB,6						
			BsF TRISB,5
			BsF TRISB,4
			BsF TRISB,2
			BsF TRISB,1      		
			GOTO	VUAusFertig
			
AusgTest3	DECF   vVUistBank1,F
			BTFSS  STATUS,Z
			GOTO   AusgTest4
			BCF TRISB,6

			BsF TRISB,7						
			BsF TRISB,5
			BsF TRISB,4
			BsF TRISB,2
			BsF TRISB,1
			GOTO	VUAusFertig

AusgTest4	DECF   vVUistBank1,F
			BTFSS  STATUS,Z
			GOTO   AusgTest5
			BCF TRISB,5

			BsF TRISB,6						
			BsF TRISB,4
			BsF TRISB,7
			BsF TRISB,2
			BsF TRISB,1
			GOTO	VUAusFertig

AusgTest5	DECF   vVUistBank1,F
			BTFSS  STATUS,Z
			GOTO   AusgTest6
			BCF TRISB,4

			BsF TRISB,5						
			BsF TRISB,2
			BsF TRISB,6
			BsF TRISB,1
			BsF TRISB,7
			GOTO	VUAusFertig

AusgTest6	DECF   vVUistBank1,F
			BTFSS  STATUS,Z
			GOTO   AusgTest7
			BCF TRISB,2

			BsF TRISB,4						
			BsF TRISB,1
			BsF TRISB,5
			BsF TRISB,6
			BsF TRISB,7
			GOTO	VUAusFertig

AusgTest7	BCF TRISB,1

			BsF TRISB,2						
			BsF TRISB,4
			BsF TRISB,5
			BsF TRISB,6
			BsF TRISB,7
			
			
VUAusFertig		BCF  STATUS,RP0
			CALL Schl100   ; damit die Änderung Zeit hat wirksam zu werden und nicht automatisch als zwei Pegelklassen registriert wird
								;soll überflüssiges Hin und Her-Regeln reduzieren
   RETURN

;XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
;Testprogramm für Led an RA3:     (Einfach CALL TestBlk an der entsprechenden Programmstelle einfügen LED an Diagnoseausgang (oder Sendestufe hören)
								  ;(  Semikolons entfernen und schon kann man am Toggeln feststellen ob das Programm an der Stelle auch vorbeikommt und wann.)
		
;TestBlk	BTFSS vMyFlags,3   ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	GOTO Mu3Set     ; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	BCF vMyFlags,3    ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	BCF  PORTA,3			; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	GOTO BlinkFert	; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;Mu3Set	BsF vMyFlags,3	; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;		BSF PORTA,3		; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;BlinkFert RETURN        ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


;TestBlk1 BCF STATUS,RP0          ;Variante bestimmt zur Einfügung an Bank-1-Stellen.)
;	BTFSS vMyFlags,3   ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	GOTO Mu3Set1     ; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	BCF vMyFlags,3    ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	BCF  PORTA,3			; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;	GOTO BlinkFert1	; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;Mu3Set1	BsF vMyFlags,3	; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;		BSF PORTA,3		; wieder entfernen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;BlinkFert1		BSF STATUS,RP0
; RETURN        ;nur zum TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



        END



