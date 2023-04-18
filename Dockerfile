FROM gcc

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
CMD ["./grade.sh"]