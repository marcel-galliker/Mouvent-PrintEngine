$ *****************************************************************************
$ File:     @(#)$Id: pcl3opts-de.msg,v 1.9 2001/05/31 15:23:28 Martin Rel $
$ Contents: NLS message catalogue for pcl3opts in German
$ Author:   Martin Lottermoser, Greifswaldstrasse 28, 38124 Braunschweig,
$           Germany
$ 
$ *****************************************************************************
$									      *
$	Copyright (C) 1999, 2000, 2001 by Martin Lottermoser		      *
$	All rights reserved						      *
$									      *
$ *****************************************************************************

$set 2
$quote "
1	"Aufruf: %s [<Optionen>] [<Eingabedateien>].\n  <Option> ::= -o <Ausgabedatei>\n"
2	"Die Datei `%s' lie? sich nicht zum Schreiben ?ffnen:\n  %s.\n"
3	"Die Eingabedatei `%s' lie? sich nicht ?ffnen:\n  %s.\n"
4	"Fehler beim Schlie?en von `%s':\n  %s.\n"
9	"Fehler bei Speicherallokation: %s.\n"
10	"\nDer vorige Abschnitt mit Rasterdaten wird gefolgt von einem mit einer\nabweichenden Konfiguration.\n"
11	"Ich habe einen Abschnitt von Rasterdaten gefunden, der eine Kombination von\nAufl?sungen und Intensit?tsstufen verwendet, die pcl3 nicht reproduzieren kann:\n"
12	"    Komponente %d: %4d x %4d ppi %2d Stufen\n"
13	"Meine beste Vermutung f?r die zum Erzeugen einer ?hnlichen Datei n?tige\nKommandozeile ist:\n"
14	"Diese Datei verwendet einen PCL Code f?r die Seitengr??e (%d),\n  der pcl3 unbekannt ist.\n"
15	"Die Seitengr??e ist %s"
16	"mm"
17	"Das ausgew?hlte Medienziel (Ausgabefach) ist %d.\n"
18	"Um eine Medienposition (Ein- oder Ausgabe) ausw?hlen zu k?nnen, m?ssen Sie die\nVerzeichnisse `InputAttributes' oder `OutputAttributes' entsprechend\nkonfigurieren. Schlagen Sie dazu in der Handbuchseite gs-pcl3(1) nach.\n"
19	"Die ausgew?hlte Medienquelle (Eingabefach) ist %d.\n"
20	"Ung?ltige Feldl?nge im Kommando Configure Raster Data.\n"
30	"Diese Datei verwendet mindestens eine Kompressionsmethode f?r\n  Rasterdaten (%d), die pcl3 nicht bekannt ist.\n"
31	"Diese Datei enth?lt ein Configure Raster Data Kommando mit einer\n  ung?ltigen Anzahl von Farbkomponenten (%d) und den folgenden Daten:\n"
32	"Diese Datei enth?lt ein Configure Raster Data Kommando mit einem\n  unbekannten Format (%d).\n"
33	"  Es *k?nnte* das folgende bedeuten:\n"
34	"  Dieses Format wird von pcl3 nicht unterst?tzt.\n"
$ Message 40 should not be used.
41	"Diese Datei verwendet eine Farbpalette (%d), die pcl3 nicht unterst?tzt.\n"
50	"Dies scheint keine Datei mit PCL-Rasterdaten zu sein.\n"
60	"Ich habe einen unbekannten Wert f?r das Kommando zur Auswahl von\nSimplex/Duplex-Druck gefunden: %d.\n"
70	"Obwohl das Farbmodell in dieser Datei als CMYK deklariert ist, wird keine\nschwarze Farbe verwendet. Sollten Sie feststellen, dass der Drucker eine mit\nder obigen Kommandozeile generierte Datei nicht akzeptiert, versuchen Sie es\nstattdessen mit dem Farbmodell CMY.\n\n"
$ Message number 80 is in use.
