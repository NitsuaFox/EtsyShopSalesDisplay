# EtsyShopSalesDisplay - Without using Etsy API and oAuth shite.
=================================================================
Scrapes Etsy web store URL for Sales in top bar using a google sheet script. 
Google Sheet is then read by ESP32 script and displayed on I2C Liquid Crystal Display 
Alerts whena new sale is detected, and updates Last Sold timestamp which is also displayed. 

1. Import INO C++ on your board on your board - this ib based on ESP32 board.
2. Create EXACT same sheet as pictured "Image 08-03-2023 at 17.33.jpg"  and add your shop URL(s) in Colum A to get your TOTAL sales in B12. (that's what gets scrapped) 
3. Open Google Sheets script up on your google sheets spreadsheet and copy in the Google Script. 
4. Set up a trigger for 'UpdateSales' to update every 1 Minute. (This will scrape the URLS for sales every 1 minute)
5. For the ESP to download the google sheet data it needs to be published (not shared to everyone) published.
  +File>share>publish-to-web.
6. Then update the URL created by this (It should look like a shit web page from the 1990's. Add it to your INO script where it says add google sheet.
Remember to add your WiFi SSID/Password.

First project on ESP and using Arduino. I had ChatGPT help me a ton so if it looks like shit code, blamae 'that' but it works! 

Cheers
