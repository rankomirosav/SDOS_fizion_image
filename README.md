# SDOS_fizion_image

Za izradu projekta je korišten Cross Core Embedded Studio, koje predstavlja inregrisano razvojno
za rad sa DSP procesorima proizvođača Analog Devices. Za ovaj prokjekat je korišten 
 ADSP-21489 signal procesor iz porodice SHARC procesora. Dodatne informacije o zadatku se
mogu pronaći u Izvjestaju.pdf . Da bi se uspješno kompajlirao i izvršio ovaj zadatka potrebno je 
prvo pokrenuti python skriptu, koja će da ulazne slike konvertuje u header fajl koji sadrži 
informacije od značaja kao što su širina i visina slike, te RGB vrijednosti piksela.
Python skripta kon_image_in_header.py se da izvrši ovo konvertovanje, nakon čega je potrebno samo 
ukljušiti date hedere u kod, vašno je napomenuti da je potrebno tokom izvršavanja koda uključiti 
samo jedan par header-a.
Nakon što se generiše slika izlaza, tj. slika u punom fokusu, vrijednosti piksela te njena 
visina i širina se unose u izlazni .h fajl. Kada se generiše ovaj fajl onda je  potrebno pokrenuti 
drugu python skriptu koja ce pročita vrijednosti tog .h fajla i na osnovu pročitanih podataka da 
formira sliku izlaza.
Python skripte za rad sa slikama koriste python Image biblioteku.  