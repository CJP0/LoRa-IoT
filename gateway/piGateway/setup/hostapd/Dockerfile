FROM debian
RUN apt-get update
RUN apt-get install -y hostapd
ADD hostapd.conf /etc/hostapd/
RUN sed -i 's/#DAEMON_CONF=""/DAEMON_CONF="\/etc\/hostapd\/hostapd.conf"/g' /etc/default/hostapd
CMD ["hostapd","-dd","/etc/hostapd/hostapd.conf"]
