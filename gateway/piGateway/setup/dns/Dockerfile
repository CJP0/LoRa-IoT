FROM debian
RUN apt-get update
RUN apt-get install -y dnsmasq
ADD resolv.dnsmasq /etc
ADD dnsmasq.conf /etc
CMD ["dnsmasq","--keep-in-foreground"]
